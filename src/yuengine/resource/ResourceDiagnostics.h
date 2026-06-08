#pragma once

#include "yuengine/resource/VirtualFileSystem.h"

#include <filesystem>
#include <string>
#include <vector>

namespace yu::resource {

struct ResourceCheckReport {
    std::string kind;
    std::string query;
    std::string purpose;
    bool found = false;
    std::vector<ResourceResolution> resolutions;
};

struct ScriptResourceReport {
    std::string module;
    std::vector<ResourceCheckReport> resourceRefs;
};

struct ResourceReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    size_t looseMounts = 0;
    size_t packResources = 0;
    std::vector<ResourceCheckReport> requiredResources;
    std::vector<ScriptResourceReport> scriptModules;
};

ResourceReport inspectProjectResources(const std::filesystem::path& manifestPath);
std::string resourceReportToJson(const ResourceReport& report);

} // namespace yu::resource
