#include "yuengine/project/ProjectManifest.h"

#include "yuengine/core/Json.h"

#include <stdexcept>

namespace yu::project {
namespace {

std::string requireString(const core::JsonValue& object, const std::string& key)
{
    const auto& value = object.at(key);
    if (!value.isString()) {
        throw std::runtime_error("manifest field must be string: " + key);
    }
    return value.asString();
}

std::string optionalString(const core::JsonValue& object, const std::string& key)
{
    const auto& value = object.getOrNull(key);
    return value.isString() ? value.asString() : std::string();
}

void appendOptionalStringArray(
    const core::JsonValue& object,
    const std::string& key,
    std::vector<std::string>& output)
{
    const auto& value = object.getOrNull(key);
    if (!value.isArray()) {
        return;
    }
    for (const auto& item : value.asArray()) {
        if (!item.isString()) {
            throw std::runtime_error("manifest array field must contain strings: " + key);
        }
        output.push_back(item.asString());
    }
}

} // namespace

std::filesystem::path resolveProjectPath(const std::filesystem::path& projectRoot, const std::string& value)
{
    const std::filesystem::path path = projectRoot / value;
    std::error_code error;
    const auto resolved = std::filesystem::weakly_canonical(path, error);
    return error ? std::filesystem::absolute(path) : resolved;
}

ProjectManifest loadProjectManifest(const std::filesystem::path& path)
{
    ProjectManifest manifest;
    std::error_code error;
    manifest.manifestPath = std::filesystem::weakly_canonical(path, error);
    if (error) {
        manifest.manifestPath = std::filesystem::absolute(path);
    }
    manifest.projectRoot = manifest.manifestPath.parent_path();

    core::JsonValue root = core::parseJson(core::readTextFile(manifest.manifestPath.string()));
    if (!root.isObject()) {
        throw std::runtime_error("project manifest root must be object");
    }

    manifest.schema = requireString(root, "schema");
    manifest.projectId = requireString(root, "project_id");
    manifest.displayName = requireString(root, "display_name");
    manifest.role = requireString(root, "role");
    manifest.engineProfile = requireString(root, "engine_profile");

    const auto& language = root.at("language");
    manifest.defaultLanguage = requireString(language, "default");
    for (const auto& item : language.at("supported").asArray()) {
        manifest.supportedLanguages.push_back(item.asString());
    }

    for (const auto& item : root.at("mounts").asArray()) {
        Mount mount;
        mount.id = optionalString(item, "id");
        mount.type = requireString(item, "type");
        mount.path = resolveProjectPath(manifest.projectRoot, requireString(item, "path"));
        manifest.mounts.push_back(std::move(mount));
    }

    for (const auto& item : root.at("script_roots").asArray()) {
        ScriptRoot scriptRoot;
        scriptRoot.type = requireString(item, "type");
        scriptRoot.path = resolveProjectPath(manifest.projectRoot, requireString(item, "path"));
        manifest.scriptRoots.push_back(std::move(scriptRoot));
    }

    const auto& startup = root.at("startup");
    appendOptionalStringArray(startup, "preload_scripts", manifest.startup.preloadScripts);
    appendOptionalStringArray(startup, "dependency_scripts", manifest.startup.dependencyScripts);
    manifest.startup.entryModule = requireString(startup, "entry_module");
    manifest.startup.entryFunction = requireString(startup, "entry_function");

    for (const auto& [key, value] : root.at("data").asObject()) {
        if (value.isNull()) {
            continue;
        }
        manifest.dataPaths.push_back({key, resolveProjectPath(manifest.projectRoot, value.asString())});
    }

    const auto& runtime = root.at("runtime");
    manifest.renderer = requireString(runtime, "renderer");
    manifest.scriptVm = requireString(runtime, "script_vm");
    manifest.audio = requireString(runtime, "audio");
    manifest.savePolicy = requireString(runtime, "save_policy");

    const auto& resources = root.getOrNull("resources");
    if (resources.isObject()) {
        const auto& required = resources.getOrNull("required");
        if (required.isArray()) {
            for (const auto& item : required.asArray()) {
                RequiredResource resource;
                resource.kind = requireString(item, "kind");
                resource.path = requireString(item, "path");
                resource.purpose = optionalString(item, "purpose");
                manifest.requiredResources.push_back(std::move(resource));
            }
        }
    }

    const auto& oracle = root.getOrNull("oracle");
    if (oracle.isObject()) {
        manifest.oracle.titleEntryScript = optionalString(oracle, "title_entry_script");
        manifest.oracle.firstMissionCandidate = optionalString(oracle, "first_mission_candidate");
    }

    return manifest;
}

} // namespace yu::project
