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
        api.implementationStatus = parts[7];
        apis_[api.name] = std::move(api);
    }
}

const ApiSurface* NativeRegistry::find(const std::string& name) const
{
    auto it = apis_.find(name);
    return it == apis_.end() ? nullptr : &it->second;
}

size_t NativeRegistry::size() const
{
    return apis_.size();
}

} // namespace yu::native
