#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace yu::native {

struct ApiSurface {
    std::string name;
    std::string service;
    std::string ownerLevel;
    std::string implementationStatus;
    int documentedCalls = 0;
    std::string evidence;
};

class NativeRegistry {
public:
    void loadMarkdownSurface(const std::filesystem::path& path);
    const ApiSurface* find(const std::string& name) const;
    const std::map<std::string, ApiSurface>& apis() const;
    std::map<std::string, int> serviceApiCounts() const;
    std::map<std::string, int> serviceCallCounts() const;
    size_t implementationStatusCount(const std::string& status) const;
    size_t unownedCount() const;
    size_t size() const;

private:
    std::map<std::string, ApiSurface> apis_;
};

} // namespace yu::native
