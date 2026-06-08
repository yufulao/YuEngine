#include "yuengine/script/SqasmModule.h"

#include <algorithm>
#include <fstream>
#include <regex>
#include <set>
#include <stdexcept>

namespace yu::script {
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

std::string stripQuotes(std::string value)
{
    value = trim(value);
    if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
        return value.substr(1, value.size() - 2);
    }
    return value == "None" ? std::string() : value;
}

bool looksLikeResource(const std::string& value)
{
    if (value.find('/') == std::string::npos && value.find('\\') == std::string::npos) {
        return false;
    }
    if (value.rfind("../", 0) == 0) {
        return false;
    }
    return true;
}

} // namespace

SqasmModule loadSqasmModule(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("failed to read sqasm module: " + path.string());
    }

    SqasmModule module;
    module.path = path;
    SqasmFunction* current = nullptr;
    std::set<std::string> resourceRefs;

    const std::regex functionRegex(R"(^\s*function\s+#([0-9]+)\s+('[^']*'|None)\s+source=('[^']*'|\S+)\s+offset=(0x[0-9a-fA-F]+))");
    const std::regex instructionRegex(R"(^\s*([0-9]+)\s+L([0-9]+)\s+(\S+).*)");
    const std::regex literalRegex(R"(literal\[[0-9]+\]='([^']*)')");
    const std::regex inputRegex(R"(^;\s*input:\s*(.+)$)");

    std::string line;
    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, inputRegex)) {
            module.input = trim(match[1].str());
            continue;
        }
        if (std::regex_search(line, match, functionRegex)) {
            SqasmFunction function;
            function.ordinal = std::stoi(match[1].str());
            function.name = stripQuotes(match[2].str());
            function.source = stripQuotes(match[3].str());
            function.offset = match[4].str();
            module.functions.push_back(std::move(function));
            current = &module.functions.back();
            continue;
        }
        if (!current) {
            continue;
        }
        if (std::regex_search(line, match, instructionRegex)) {
            ++module.instructionCount;
            ++current->instructionCount;
            const int pc = std::stoi(match[1].str());
            const int sourceLine = std::stoi(match[2].str());
            const std::string op = match[3].str();
            std::smatch literalMatch;
            if (std::regex_search(line, literalMatch, literalRegex)) {
                std::string literal = literalMatch[1].str();
                if (looksLikeResource(literal)) {
                    resourceRefs.insert(literal);
                }
                if (op.rfind("_OP_PREPCALL", 0) == 0) {
                    current->calls.push_back({literal, sourceLine, pc});
                    ++module.callCount;
                }
            }
        }
    }

    module.resourceRefs.assign(resourceRefs.begin(), resourceRefs.end());
    return module;
}

std::filesystem::path resolveScriptModule(const std::vector<std::filesystem::path>& roots, const std::string& module)
{
    for (const auto& root : roots) {
        std::vector<std::filesystem::path> candidates;
        candidates.push_back(root / module);
        candidates.push_back(root / (module + ".sqasm"));
        if (module.ends_with(".b64")) {
            candidates.push_back(root / (module + ".sqasm"));
        }
        for (const auto& candidate : candidates) {
            if (std::filesystem::exists(candidate)) {
                return std::filesystem::weakly_canonical(candidate);
            }
        }
    }
    return {};
}

} // namespace yu::script
