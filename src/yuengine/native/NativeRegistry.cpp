#include "yuengine/native/NativeRegistry.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace yu::native {
namespace {

std::string trim(const std::string& value)
{
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

std::vector<std::string> splitPipe(const std::string& line)
{
    std::vector<std::string> parts;
    std::stringstream stream(line);
    std::string part;
    while (std::getline(stream, part, '|')) {
        parts.push_back(trim(part));
    }
    return parts;
}

std::string stripBackticks(std::string value)
{
    value = trim(value);
    if (value.size() >= 2 && value.front() == '`' && value.back() == '`') {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

int parseInt(const std::string& value)
{
    try {
        return std::stoi(value);
    } catch (...) {
        return 0;
    }
}

} // namespace

void NativeRegistry::loadMarkdownSurface(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("failed to read native surface markdown: " + path.string());
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("| `", 0) != 0) {
            continue;
        }
        auto parts = splitPipe(line);
        if (parts.size() < 10) {
            continue;
        }
        ApiSurface api;
        api.name = stripBackticks(parts[1]);
        api.service = parts[2];
        api.ownerLevel = parts[3];
        api.documentedCalls = parseInt(parts[4]);
        api.evidence = parts[6];
        api.implementationStatus = parts[7];
        apis_[api.name] = std::move(api);
    }
}

const ApiSurface* NativeRegistry::find(const std::string& name) const
{
    auto it = apis_.find(name);
    return it == apis_.end() ? nullptr : &it->second;
}

const std::map<std::string, ApiSurface>& NativeRegistry::apis() const
{
    return apis_;
}

std::map<std::string, int> NativeRegistry::serviceApiCounts() const
{
    std::map<std::string, int> counts;
    for (const auto& [_, api] : apis_) {
        ++counts[api.service];
    }
    return counts;
}

std::map<std::string, int> NativeRegistry::serviceCallCounts() const
{
    std::map<std::string, int> counts;
    for (const auto& [_, api] : apis_) {
        counts[api.service] += api.documentedCalls;
    }
    return counts;
}

size_t NativeRegistry::implementationStatusCount(const std::string& status) const
{
    size_t count = 0;
    for (const auto& [_, api] : apis_) {
        if (api.implementationStatus == status) {
            ++count;
        }
    }
    return count;
}

size_t NativeRegistry::unownedCount() const
{
    size_t count = 0;
    for (const auto& [_, api] : apis_) {
        if (api.service.empty() || api.service == "Unassigned" || api.service == "unassigned") {
            ++count;
        }
    }
    return count;
}

size_t NativeRegistry::size() const
{
    return apis_.size();
}

} // namespace yu::native
