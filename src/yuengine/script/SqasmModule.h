#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace yu::script {

struct SqasmCall {
    std::string name;
    int sourceLine = -1;
    int pc = -1;
};

struct SqasmFunction {
    int ordinal = -1;
    std::string name;
    std::string source;
    std::string offset;
    int instructionCount = 0;
    std::vector<SqasmCall> calls;
};

struct SqasmModule {
    std::filesystem::path path;
    std::string input;
    std::vector<SqasmFunction> functions;
    std::vector<std::string> resourceRefs;
    int instructionCount = 0;
    int callCount = 0;
};

SqasmModule loadSqasmModule(const std::filesystem::path& path);
std::filesystem::path resolveScriptModule(const std::vector<std::filesystem::path>& roots, const std::string& module);

} // namespace yu::script
