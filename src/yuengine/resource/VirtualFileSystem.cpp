#include "yuengine/resource/VirtualFileSystem.h"

#include "yuengine/core/Json.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>

namespace yu::resource {
namespace {

bool startsWith(const std::string& value, const std::string& prefix)
{
    return value.rfind(prefix, 0) == 0;
}

int64_t optionalInt64(const core::JsonValue& object, const std::string& key)
{
    const auto& value = object.getOrNull(key);
    return value.isNumber() ? static_cast<int64_t>(value.asNumber()) : -1;
}

std::string optionalString(const core::JsonValue& object, const std::string& key)
{
    const auto& value = object.getOrNull(key);
    return value.isString() ? value.asString() : std::string();
}

int64_t fileSizeOrUnknown(const std::filesystem::path& path)
{
    std::error_code error;
    const auto size = std::filesystem::file_size(path, error);
    return error ? -1 : static_cast<int64_t>(size);
}

std::vector<std::byte> readAllBytes(std::ifstream& stream)
{
    std::vector<std::byte> bytes;
    for (std::istreambuf_iterator<char> it(stream), end; it != end; ++it) {
        bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(*it)));
    }
    return bytes;
}

std::filesystem::path stablePhysicalPath(const std::filesystem::path& path)
{
    std::error_code error;
    const auto resolved = std::filesystem::weakly_canonical(path, error);
    return error ? std::filesystem::absolute(path) : resolved;
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
                    ResourceEntry entry;
                    entry.virtualPath = normalizeResourcePath(path.asString());
                    entry.mountType = "pack_manifest";
                    entry.pack = optionalString(item, "pack");
                    entry.size = optionalInt64(item, "size");
                    entry.relativeOffset = optionalInt64(item, "relative_offset");
                    entry.absoluteOffset = optionalInt64(item, "absolute_offset");
                    packResources_[entry.virtualPath] = std::move(entry);
                }
            }
        }
    }
}

ResourceResolution VirtualFileSystem::resolvePath(const std::string& path) const
{
    const std::string normalized = normalizeResourcePath(path);
    auto pack = packResources_.find(normalized);
    if (pack != packResources_.end()) {
        return {true, pack->second};
    }
    for (const auto& root : looseRoots_) {
        std::filesystem::path physical = root / path;
        if (std::filesystem::exists(physical)) {
            ResourceEntry entry;
            entry.virtualPath = normalized;
            entry.mountType = "loose";
            entry.physicalPath = stablePhysicalPath(physical);
            entry.size = fileSizeOrUnknown(physical);
            return {true, entry};
        }
    }
    return {};
}

std::vector<ResourceResolution> VirtualFileSystem::resolveStem(const std::string& stem) const
{
    std::vector<ResourceResolution> results;
    const std::string normalized = normalizeResourcePath(stem);
    for (const auto& [path, entry] : packResources_) {
        if (startsWith(path, normalized)) {
            results.push_back({true, entry});
        }
    }
    for (const auto& root : looseRoots_) {
        std::filesystem::path parent = root / std::filesystem::path(stem).parent_path();
        if (!std::filesystem::exists(parent)) {
            continue;
        }
        std::string name = normalizeResourcePath(std::filesystem::path(stem).filename().string());
        for (const auto& entry : std::filesystem::directory_iterator(parent)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            std::string filename = entry.path().filename().string();
            if (normalizeResourcePath(filename).rfind(name, 0) == 0) {
                ResourceEntry resource;
                resource.virtualPath = normalizeResourcePath((std::filesystem::path(stem).parent_path() / filename).string());
                resource.mountType = "loose";
                resource.physicalPath = stablePhysicalPath(entry.path());
                resource.size = fileSizeOrUnknown(entry.path());
                results.push_back({true, resource});
            }
        }
    }
    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
        return a.entry.virtualPath < b.entry.virtualPath;
    });
    return results;
}

ResourceBytes VirtualFileSystem::readBytes(const std::string& path) const
{
    const std::string normalized = normalizeResourcePath(path);
    for (const auto& root : looseRoots_) {
        std::filesystem::path physical = root / path;
        if (!std::filesystem::exists(physical)) {
            physical = root / normalized;
        }
        if (!std::filesystem::exists(physical)) {
            continue;
        }

        std::ifstream stream(physical, std::ios::binary);
        if (!stream) {
            continue;
        }
        ResourceBytes out;
        out.found = true;
        out.virtualPath = normalized;
        out.physicalPath = stablePhysicalPath(physical);
        out.bytes = readAllBytes(stream);
        return out;
    }

    const auto resolution = resolvePath(path);
    if (resolution.found && !resolution.entry.physicalPath.empty()) {
        std::ifstream stream(resolution.entry.physicalPath, std::ios::binary);
        if (stream) {
            ResourceBytes out;
            out.found = true;
            out.virtualPath = resolution.entry.virtualPath;
            out.physicalPath = resolution.entry.physicalPath;
            out.bytes = readAllBytes(stream);
            return out;
        }
    }

    return {};
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
