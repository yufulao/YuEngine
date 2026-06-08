#pragma once

#include <filesystem>
#include <map>
#include <string>

namespace yu::native {

struct ApiSurface {
    std::string name;
    std::string service;
    std::string ownerLevel;
    std::string implementationStatus;
};

class NativeRegistry {
public:
    void loadMarkdownSurface(const std::filesystem::path& path);
    const ApiSurface* find(const std::string& name) const;
    size_t size() const;

private:
    std::map<std::string, ApiSurface> apis_;
};

} // namespace yu::native
