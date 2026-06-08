#pragma once

#include "yuengine/project/ProjectManifest.h"

#include <filesystem>
#include <set>
#include <string>
#include <vector>

namespace yu::resource {

struct ResourceResolution {
    bool found = false;
    std::string mountType;
    std::filesystem::path physicalPath;
    std::string virtualPath;
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
    std::set<std::string> packResources_;
};

std::string normalizeResourcePath(const std::string& path);

} // namespace yu::resource
