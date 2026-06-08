#pragma once

#include "yuengine/project/ProjectManifest.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace yu::resource {

struct ResourceEntry {
    std::string virtualPath;
    std::string mountType;
    std::filesystem::path physicalPath;
    std::string pack;
    int64_t size = -1;
    int64_t relativeOffset = -1;
    int64_t absoluteOffset = -1;
};

struct ResourceResolution {
    bool found = false;
    ResourceEntry entry;
};

class VirtualFileSystem {
public:
    void mountProject(const project::ProjectManifest& manifest);
    ResourceResolution resolvePath(const std::string& path) const;
    std::vector<ResourceResolution> resolveStem(const std::string& stem) const;
    size_t packResourceCount() const;
    size_t looseMountCount() const;

private:
    std::vector<std::filesystem::path> looseRoots_;
    std::map<std::string, ResourceEntry> packResources_;
};

std::string normalizeResourcePath(const std::string& path);

} // namespace yu::resource
