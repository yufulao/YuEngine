#include "yuengine/resource/VirtualFileSystem.h"

#include "yuengine/core/Json.h"

#include <algorithm>

namespace yu::resource {
namespace {

bool startsWith(const std::string& value, const std::string& prefix)
{
    return value.rfind(prefix, 0) == 0;
}

} // namespace

std::string normalizeResourcePath(const std::string& path)
{
    std::string out = path;
    std::replace(out.begin(), out.end(), '\\', '/');
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

void VirtualFileSystem::mountProject(const project::ProjectManifest& manifest)
{
    looseRoots_.clear();
    packResources_.clear();
    for (const auto& mount : manifest.mounts) {
        if (mount.type == "loose") {
            looseRoots_.push_back(mount.path);
            continue;
        }
        if (mount.type == "pack_manifest") {
            core::JsonValue root = core::parseJson(core::readTextFile(mount.path.string()));
            for (const auto& item : root.asArray()) {
                const auto& path = item.getOrNull("path");
                if (path.isString()) {
                    packResources_.insert(normalizeResourcePath(path.asString()));
                }
            }
        }
    }
}

ResourceResolution VirtualFileSystem::resolvePath(const std::string& path) const
{
    const std::string normalized = normalizeResourcePath(path);
    if (packResources_.contains(normalized)) {
        return {true, "pack_manifest", {}, normalized};
    }
    for (const auto& root : looseRoots_) {
        std::filesystem::path physical = root / path;
        if (std::filesystem::exists(physical)) {
            return {true, "loose", physical, normalized};
        }
    }
    return {};
}

std::vector<ResourceResolution> VirtualFileSystem::resolveStem(const std::string& stem) const
{
    std::vector<ResourceResolution> results;
    const std::string normalized = normalizeResourcePath(stem);
    for (const auto& path : packResources_) {
        if (startsWith(path, normalized)) {
            results.push_back({true, "pack_manifest", {}, path});
        }
    }
    for (const auto& root : looseRoots_) {
        std::filesystem::path parent = root / std::filesystem::path(stem).parent_path();
        if (!std::filesystem::exists(parent)) {
            continue;
        }
        std::string name = std::filesystem::path(stem).filename().string();
        for (const auto& entry : std::filesystem::directory_iterator(parent)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            std::string filename = entry.path().filename().string();
            if (filename.rfind(name, 0) == 0) {
                results.push_back({true, "loose", entry.path(), normalizeResourcePath((std::filesystem::path(stem).parent_path() / filename).string())});
            }
        }
    }
    return results;
}

size_t VirtualFileSystem::packResourceCount() const
{
    return packResources_.size();
}

size_t VirtualFileSystem::looseMountCount() const
{
    return looseRoots_.size();
}

} // namespace yu::resource
