#include "yuengine/runtime/EngineRuntime.h"

#include "yuengine/core/Json.h"
#include "yuengine/native/NativeRegistry.h"
#include "yuengine/native/NativeServices.h"
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

void loadModule(
    BootReport& report,
    const std::vector<std::filesystem::path>& roots,
    const std::string& moduleName,
    const native::NativeRegistry& registry,
    const native::NativeServiceCatalog& serviceCatalog,
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
            const auto dispatch = serviceCatalog.dispatch(*surface, {
                module.path.string(),
                function.name,
                call.sourceLine,
                call.pc,
            });
            auto& obligation = obligations[call.name];
            obligation.api = call.name;
            obligation.service = dispatch.service;
            obligation.status = dispatch.implementationStatus;
            ++obligation.calls;
        }
    }
}

size_t failedPhaseCount(const BootReport& report)
{
    size_t count = 0;
    for (const auto& phase : report.phases) {
        if (phase.status != "ok") {
            ++count;
        }
    }
    return count;
}

} // namespace

BootReport bootProject(const std::filesystem::path& manifestPath, const std::filesystem::path& repoRoot)
{
    BootReport report;
    RuntimeContext context;
    try {
        traceBoot("enter boot project");

        traceBoot("load manifest");
        context.services().setManifest(project::loadProjectManifest(manifestPath));
        const auto& manifest = context.services().manifest();
        report.projectId = manifest.projectId;
        context.recordPhase("load_project_manifest", "ok", manifest.projectId);

        traceBoot("mount vfs");
        context.services().vfs().mountProject(manifest);
        report.looseMounts = context.services().vfs().looseMountCount();
        report.packResources = context.services().vfs().packResourceCount();
        context.recordPhase(
            "mount_vfs",
            "ok",
            "loose=" + std::to_string(report.looseMounts) + " pack=" + std::to_string(report.packResources));

        traceBoot("verify required resources");
        int missingResources = 0;
        for (const auto& required : manifest.requiredResources) {
            if (required.kind == "path" && !context.services().vfs().resolvePath(required.path).found) {
                addError(report, "required resource not resolved: " + required.path);
                ++missingResources;
            }
            if (required.kind == "stem" && context.services().vfs().resolveStem(required.path).empty()) {
                addError(report, "required resource stem not resolved: " + required.path);
                ++missingResources;
            }
        }
        context.recordPhase(
            "verify_required_resources",
            missingResources == 0 ? "ok" : "failed",
            "required=" + std::to_string(manifest.requiredResources.size()) + " missing="
                + std::to_string(missingResources));

        traceBoot("load native registry");
        std::filesystem::path surfacePath = repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md";
        if (std::filesystem::exists(surfacePath)) {
            context.services().nativeRegistry().loadMarkdownSurface(surfacePath);
        } else {
            report.warnings.push_back("native surface markdown not found: " + surfacePath.string());
        }
        report.nativeApis = context.services().nativeRegistry().size();
        context.recordPhase("load_native_registry", "ok", "apis=" + std::to_string(report.nativeApis));

        traceBoot("bind native services");
        report.nativeServices = context.services().nativeServices().size();
        const auto unboundApis =
            context.services().nativeServices().unboundApis(context.services().nativeRegistry());
        report.unboundNativeApis = unboundApis.size();
        if (!unboundApis.empty()) {
            addError(report, "native APIs have no service interface: " + std::to_string(unboundApis.size()));
        }
        context.recordPhase(
            "bind_native_services",
            unboundApis.empty() ? "ok" : "failed",
            "services=" + std::to_string(report.nativeServices) + " unbound=" + std::to_string(unboundApis.size()));

        traceBoot("load script modules");
        std::map<std::string, ObligationReport> obligations;
        const auto errorsBeforeScripts = report.errors.size();
        auto roots = context.services().scriptRoots();
        for (const auto& preload : manifest.startup.preloadScripts) {
            loadModule(
                report,
                roots,
                preload,
                context.services().nativeRegistry(),
                context.services().nativeServices(),
                obligations);
        }
        for (const auto& dependency : manifest.startup.dependencyScripts) {
            loadModule(
                report,
                roots,
                dependency,
                context.services().nativeRegistry(),
                context.services().nativeServices(),
                obligations);
        }
        loadModule(
            report,
            roots,
            manifest.startup.entryModule,
            context.services().nativeRegistry(),
            context.services().nativeServices(),
            obligations);
        context.recordPhase(
            "load_startup_scripts",
            report.errors.size() == errorsBeforeScripts ? "ok" : "failed",
            "modules=" + std::to_string(report.modules.size()));

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
        context.recordPhase("collect_native_obligations", "ok", "apis=" + std::to_string(report.obligations.size()));
        traceBoot("boot report ready");
    } catch (const std::exception& ex) {
        addError(report, ex.what());
        context.recordPhase("boot", "failed", ex.what());
    }
    report.phases = context.phases();
    return report;
}

std::string bootReportToJson(const BootReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.boot_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false") << " phases=" << report.phases.size()
        << " failed_phases=" << failedPhaseCount(report) << " native_apis=" << report.nativeApis
        << " native_services=" << report.nativeServices << " obligations=" << report.obligations.size() << "\",\n";
    out << "  \"loose_mounts\": " << report.looseMounts << ",\n";
    out << "  \"pack_resources\": " << report.packResources << ",\n";
    out << "  \"native_apis\": " << report.nativeApis << ",\n";
    out << "  \"native_services\": " << report.nativeServices << ",\n";
    out << "  \"unbound_native_apis\": " << report.unboundNativeApis << ",\n";
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
    out << "  \"phases\": [\n";
    for (size_t i = 0; i < report.phases.size(); ++i) {
        const auto& phase = report.phases[i];
        out << "    {\"name\": \"" << core::jsonEscape(phase.name) << "\", \"status\": \""
            << core::jsonEscape(phase.status) << "\", \"detail\": \"" << core::jsonEscape(phase.detail) << "\"}";
        out << (i + 1 == report.phases.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
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
