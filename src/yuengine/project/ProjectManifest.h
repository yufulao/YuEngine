#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace yu::project {

struct Mount {
    std::string id;
    std::string type;
    std::filesystem::path path;
};

struct ScriptRoot {
    std::string type;
    std::filesystem::path path;
};

struct Startup {
    std::vector<std::string> preloadScripts;
    std::string entryModule;
    std::string entryFunction;
};

struct RequiredResource {
    std::string kind;
    std::string path;
    std::string purpose;
};

struct ProjectManifest {
    std::filesystem::path manifestPath;
    std::filesystem::path projectRoot;
    std::string schema;
    std::string projectId;
    std::string displayName;
    std::string role;
    std::string engineProfile;
    std::string defaultLanguage;
    std::vector<std::string> supportedLanguages;
    std::vector<Mount> mounts;
    std::vector<ScriptRoot> scriptRoots;
    Startup startup;
    std::vector<std::pair<std::string, std::filesystem::path>> dataPaths;
    std::string renderer;
    std::string scriptVm;
    std::string audio;
    std::string savePolicy;
    std::vector<RequiredResource> requiredResources;
};

ProjectManifest loadProjectManifest(const std::filesystem::path& path);
std::filesystem::path resolveProjectPath(const std::filesystem::path& projectRoot, const std::string& value);

} // namespace yu::project
