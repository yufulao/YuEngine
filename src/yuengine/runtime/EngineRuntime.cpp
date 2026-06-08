#include "yuengine/runtime/EngineRuntime.h"

#include "yuengine/core/Json.h"
#include "yuengine/native/NativeRegistry.h"
#include "yuengine/project/ProjectManifest.h"
#include "yuengine/resource/VirtualFileSystem.h"
#include "yuengine/script/SqasmModule.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>

namespace yu::runtime {
namespace {

void addError(BootReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

bool traceEnabled()
{
#if defined(_MSC_VER)
    char* value = nullptr;
    size_t size = 0;
    _dupenv_s(&value, &size, "YUENGINE_TRACE_BOOT");
    const bool enabled = value != nullptr && value[0] != '\0';
    std::free(value);
    return enabled;
#else
    const char* value = std::getenv("YUENGINE_TRACE_BOOT");
    return value != nullptr && value[0] != '\0';
#endif
}

void traceBoot(const std::string& phase)
{
    if (traceEnabled()) {
        std::cerr << "[yuengine boot] " << phase << std::endl;
    }
}

std::vector<std::filesystem::path> scriptRoots(const project::ProjectManifest& manifest)
{
    std::vector<std::filesystem::path> roots;
    for (const auto& root : manifest.scriptRoots) {
        roots.push_back(root.path);
    }
    return roots;
}

void loadModule(
    BootReport& report,
    const std::vector<std::filesystem::path>& roots,
    const std::string& moduleName,
    const native::NativeRegistry& registry,
    std::map<std::string, ObligationReport>& obligations)
{
    std::filesystem::path path = script::resolveScriptModule(roots, moduleName);
    if (path.empty()) {
        addError(report, "script module not found: " + moduleName);
        return;
    }
    script::SqasmModule module = script::loadSqasmModule(path);
    report.modules.push_back({
        path.string(),
        static_cast<int>(module.functions.size()),
        module.instructionCount,
        module.callCount,
        static_cast<int>(module.resourceRefs.size()),
    });
    for (const auto& function : module.functions) {
        for (const auto& call : function.calls) {
            const native::ApiSurface* surface = registry.find(call.name);
            if (!surface) {
                continue;
            }
            auto& obligation = obligations[call.name];
            obligation.api = call.name;
            obligation.service = surface->service;
            obligation.status = surface->implementationStatus.empty() ? "not_started" : surface->implementationStatus;
            ++obligation.calls;
        }
    }
}

} // namespace

BootReport bootProject(const std::filesystem::path& manifestPath, const std::filesystem::path& repoRoot)
{
    BootReport report;
    try {
        traceBoot("enter boot project");
        traceBoot("load manifest");
        project::ProjectManifest manifest = project::loadProjectManifest(manifestPath);
        report.projectId = manifest.projectId;

        traceBoot("mount vfs");
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report.looseMounts = vfs.looseMountCount();
        report.packResources = vfs.packResourceCount();

        traceBoot("verify required resources");
        for (const auto& required : manifest.requiredResources) {
            if (required.kind == "path" && !vfs.resolvePath(required.path).found) {
                addError(report, "required resource not resolved: " + required.path);
            }
            if (required.kind == "stem" && vfs.resolveStem(required.path).empty()) {
                addError(report, "required resource stem not resolved: " + required.path);
            }
        }

        traceBoot("load native registry");
        native::NativeRegistry registry;
        std::filesystem::path surfacePath = repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md";
        if (std::filesystem::exists(surfacePath)) {
            registry.loadMarkdownSurface(surfacePath);
        } else {
            report.warnings.push_back("native surface markdown not found: " + surfacePath.string());
        }
        report.nativeApis = registry.size();

        traceBoot("load script modules");
        std::map<std::string, ObligationReport> obligations;
        auto roots = scriptRoots(manifest);
        for (const auto& preload : manifest.startup.preloadScripts) {
            loadModule(report, roots, preload, registry, obligations);
        }
        loadModule(report, roots, manifest.startup.entryModule, registry, obligations);

        traceBoot("collect native obligations");
        for (const auto& [_, obligation] : obligations) {
            report.obligations.push_back(obligation);
        }
        std::sort(report.obligations.begin(), report.obligations.end(), [](const auto& a, const auto& b) {
            if (a.service == b.service) {
                return a.api < b.api;
            }
            return a.service < b.service;
        });
        traceBoot("boot report ready");
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string bootReportToJson(const BootReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.boot_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"loose_mounts\": " << report.looseMounts << ",\n";
    out << "  \"pack_resources\": " << report.packResources << ",\n";
    out << "  \"native_apis\": " << report.nativeApis << ",\n";
    out << "  \"errors\": [";
    for (size_t i = 0; i < report.errors.size(); ++i) {
        out << (i ? ", " : "") << "\"" << core::jsonEscape(report.errors[i]) << "\"";
    }
    out << "],\n";
    out << "  \"warnings\": [";
    for (size_t i = 0; i < report.warnings.size(); ++i) {
        out << (i ? ", " : "") << "\"" << core::jsonEscape(report.warnings[i]) << "\"";
    }
    out << "],\n";
    out << "  \"modules\": [\n";
    for (size_t i = 0; i < report.modules.size(); ++i) {
        const auto& module = report.modules[i];
        out << "    {\"path\": \"" << core::jsonEscape(module.path) << "\", \"functions\": " << module.functions
            << ", \"instructions\": " << module.instructions << ", \"calls\": " << module.calls
            << ", \"resource_refs\": " << module.resourceRefs << "}";
        out << (i + 1 == report.modules.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"native_obligations\": [\n";
    for (size_t i = 0; i < report.obligations.size(); ++i) {
        const auto& obligation = report.obligations[i];
        out << "    {\"api\": \"" << core::jsonEscape(obligation.api) << "\", \"service\": \""
            << core::jsonEscape(obligation.service) << "\", \"calls\": " << obligation.calls
            << ", \"status\": \"" << core::jsonEscape(obligation.status) << "\"}";
        out << (i + 1 == report.obligations.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

} // namespace yu::runtime
