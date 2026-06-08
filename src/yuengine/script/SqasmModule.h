#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace yu::script {

struct SqasmCall {
    std::string name;
    int literalIndex = -1;
    int sourceLine = -1;
    int pc = -1;
    std::string classification = "parsed-callsite";
    std::string source = "sqasm";
};

struct SqasmLiteralRef {
    std::string slot;
    int index = -1;
    std::string value;
};

struct SqasmFunctionRef {
    int index = -1;
    std::string value;
};

struct SqasmInstruction {
    int pc = -1;
    int sourceLine = -1;
    std::string op;
    std::map<std::string, int> args;
    std::string comment;
    int target = -1;
    std::vector<SqasmLiteralRef> literalRefs;
    std::vector<SqasmFunctionRef> functionRefs;
};

struct SqasmLiteral {
    int index = -1;
    std::string value;
    std::string raw;
    std::string kind;
};

struct SqasmLocal {
    std::string reg;
    std::string name;
    std::string ops;
};

struct SqasmResourceRef {
    int literalIndex = -1;
    std::string value;
    std::string kind;
};

struct SqasmClosureBinding {
    int pc = -1;
    int sourceLine = -1;
    std::string slot;
    int functionOrdinal = -1;
    std::string functionName;
};

struct SqasmFunction {
    int ordinal = -1;
    std::string name;
    std::string source;
    std::string offset;
    int stack = -1;
    bool generator = false;
    bool varargs = false;
    std::vector<std::string> parameters;
    std::vector<SqasmLiteral> literals;
    std::vector<SqasmLocal> locals;
    std::vector<SqasmInstruction> instructions;
    int instructionCount = 0;
    std::vector<SqasmCall> calls;
    std::vector<SqasmResourceRef> resourceRefs;
    std::vector<SqasmClosureBinding> closureBindings;
    std::vector<std::string> stateCandidates;
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
std::string sqasmModuleReportToJson(const SqasmModule& module);

} // namespace yu::script
