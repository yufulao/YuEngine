#include "yuengine/resource/ResourceDiagnostics.h"

#include "yuengine/core/Json.h"
#include "yuengine/project/ProjectManifest.h"
#include "yuengine/script/SqasmModule.h"

#include <algorithm>
#include <set>
#include <sstream>

namespace yu::resource {
namespace {

void addError(ResourceReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

bool hasExtension(const std::string& path)
{
    return !std::filesystem::path(path).extension().empty();
}

ResourceCheckReport resolveCheck(
    const VirtualFileSystem& vfs,
    std::string kind,
    std::string query,
    std::string purpose)
{
    ResourceCheckReport check;
    check.kind = std::move(kind);
    check.query = normalizeResourcePath(query);
    check.purpose = std::move(purpose);

    if (check.kind == "path") {
        auto resolution = vfs.resolvePath(check.query);
        check.found = resolution.found;
        if (resolution.found) {
            check.resolutions.push_back(std::move(resolution));
        }
        return check;
    }

    check.kind = "stem";
    check.resolutions = vfs.resolveStem(check.query);
    check.found = !check.resolutions.empty();
    return check;
}

std::vector<std::filesystem::path> scriptRoots(const project::ProjectManifest& manifest)
{
    std::vector<std::filesystem::path> roots;
    for (const auto& root : manifest.scriptRoots) {
        roots.push_back(root.path);
    }
    return roots;
}

void inspectScriptModule(
    ResourceReport& report,
    const VirtualFileSystem& vfs,
    const std::vector<std::filesystem::path>& roots,
    const std::string& moduleName,
    const std::string& purpose)
{
    if (moduleName.empty()) {
        return;
    }

    const std::filesystem::path modulePath = script::resolveScriptModule(roots, moduleName);
    if (modulePath.empty()) {
        addError(report, "script module not found for resource diagnostics: " + moduleName);
        return;
    }

    script::SqasmModule module = script::loadSqasmModule(modulePath);
    ScriptResourceReport moduleReport;
    moduleReport.module = modulePath.string();

    std::set<std::string> seen;
    for (const auto& ref : module.resourceRefs) {
        std::string normalized = normalizeResourcePath(ref);
        if (!seen.insert(normalized).second) {
            continue;
        }
        auto check = resolveCheck(
            vfs,
            hasExtension(normalized) ? "path" : "stem",
            normalized,
            purpose);
        if (!check.found && !hasExtension(normalized)) {
            continue;
        }
        moduleReport.resourceRefs.push_back(std::move(check));
    }

    std::sort(moduleReport.resourceRefs.begin(), moduleReport.resourceRefs.end(), [](const auto& a, const auto& b) {
        return a.query < b.query;
    });
    report.scriptModules.push_back(std::move(moduleReport));
}

void writeJsonStringArray(std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        out << (i ? ", " : "") << "\"" << core::jsonEscape(values[i]) << "\"";
    }
    out << "]";
}

void writeResolution(std::ostringstream& out, const ResourceResolution& resolution)
{
    const auto& entry = resolution.entry;
    out << "{\"virtual_path\": \"" << core::jsonEscape(entry.virtualPath) << "\", "
        << "\"mount_type\": \"" << core::jsonEscape(entry.mountType) << "\"";
    if (!entry.pack.empty()) {
        out << ", \"pack\": \"" << core::jsonEscape(entry.pack) << "\"";
    }
    if (!entry.physicalPath.empty()) {
        out << ", \"physical_path\": \"" << core::jsonEscape(entry.physicalPath.string()) << "\"";
    }
    if (entry.size >= 0) {
        out << ", \"size\": " << entry.size;
    }
    if (entry.relativeOffset >= 0) {
        out << ", \"relative_offset\": " << entry.relativeOffset;
    }
    if (entry.absoluteOffset >= 0) {
        out << ", \"absolute_offset\": " << entry.absoluteOffset;
    }
    out << "}";
}

void writeCheck(std::ostringstream& out, const ResourceCheckReport& check, int indent)
{
    const std::string pad(static_cast<size_t>(indent), ' ');
    out << pad << "{\"kind\": \"" << core::jsonEscape(check.kind) << "\", "
        << "\"query\": \"" << core::jsonEscape(check.query) << "\", "
        << "\"purpose\": \"" << core::jsonEscape(check.purpose) << "\", "
        << "\"found\": " << (check.found ? "true" : "false") << ", "
        << "\"count\": " << check.resolutions.size() << ", \"results\": [";
    for (size_t i = 0; i < check.resolutions.size(); ++i) {
        out << (i ? ", " : "");
        writeResolution(out, check.resolutions[i]);
    }
    out << "]}";
}

} // namespace

ResourceReport inspectProjectResources(const std::filesystem::path& manifestPath)
{
    ResourceReport report;
    try {
        project::ProjectManifest manifest = project::loadProjectManifest(manifestPath);
        report.projectId = manifest.projectId;

        VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report.looseMounts = vfs.looseMountCount();
        report.packResources = vfs.packResourceCount();

        for (const auto& required : manifest.requiredResources) {
            auto check = resolveCheck(vfs, required.kind, required.path, required.purpose);
            if (!check.found) {
                addError(report, "required resource not resolved: " + required.path);
            }
            report.requiredResources.push_back(std::move(check));
        }

        auto roots = scriptRoots(manifest);
        inspectScriptModule(report, vfs, roots, manifest.oracle.titleEntryScript, "oracle title resource ref");
        inspectScriptModule(report, vfs, roots, manifest.oracle.firstMissionCandidate, "oracle first mission resource ref");
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string resourceReportToJson(const ResourceReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.resource_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"loose_mounts\": " << report.looseMounts << ",\n";
    out << "  \"pack_resources\": " << report.packResources << ",\n";
    out << "  \"errors\": ";
    writeJsonStringArray(out, report.errors);
    out << ",\n";
    out << "  \"required_resources\": [\n";
    for (size_t i = 0; i < report.requiredResources.size(); ++i) {
        writeCheck(out, report.requiredResources[i], 4);
        out << (i + 1 == report.requiredResources.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"script_modules\": [\n";
    for (size_t i = 0; i < report.scriptModules.size(); ++i) {
        const auto& module = report.scriptModules[i];
        out << "    {\"module\": \"" << core::jsonEscape(module.module) << "\", \"resource_refs\": [\n";
        for (size_t j = 0; j < module.resourceRefs.size(); ++j) {
            writeCheck(out, module.resourceRefs[j], 6);
            out << (j + 1 == module.resourceRefs.size() ? "\n" : ",\n");
        }
        out << "    ]}";
        out << (i + 1 == report.scriptModules.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

} // namespace yu::resource
