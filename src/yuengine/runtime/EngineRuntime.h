#pragma once

#include "yuengine/runtime/RuntimeContext.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace yu::runtime {

struct ModuleReport {
    std::string path;
    int functions = 0;
    int instructions = 0;
    int calls = 0;
    int resourceRefs = 0;
};

struct ObligationReport {
    std::string api;
    std::string service;
    int calls = 0;
    std::string status;
};

struct BootReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    std::vector<BootPhaseReport> phases;
    std::vector<ModuleReport> modules;
    std::vector<ObligationReport> obligations;
    size_t looseMounts = 0;
    size_t packResources = 0;
    size_t nativeApis = 0;
    size_t nativeServices = 0;
    size_t unboundNativeApis = 0;
};

BootReport bootProject(const std::filesystem::path& manifestPath, const std::filesystem::path& repoRoot);
std::string bootReportToJson(const BootReport& report);

} // namespace yu::runtime
