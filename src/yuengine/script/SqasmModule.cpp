#include "yuengine/script/SqasmModule.h"

#include "yuengine/core/Json.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
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

bool parseBool(const std::string& value)
{
    return value == "True" || value == "true";
}

bool looksLikeResource(const std::string& value)
{
    if (value.empty() || value.front() == '/') {
        return false;
    }
    if (value.find('/') == std::string::npos) {
        return false;
    }
    if (value.rfind("../", 0) == 0 || value.rfind("..\\", 0) == 0) {
        return false;
    }
    for (unsigned char c : value) {
        if (std::isspace(c) || std::iscntrl(c)) {
            return false;
        }
    }
    return true;
}

std::string resourceKind(const std::string& value)
{
    if (!looksLikeResource(value)) {
        return {};
    }
    const size_t slash = value.find_last_of("/\\");
    const size_t dot = value.find_last_of('.');
    auto extension = dot != std::string::npos && (slash == std::string::npos || dot > slash) ? value.substr(dot)
                                                                                              : std::string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    if (extension == ".b64" || extension == ".dds" || extension == ".fmp" || extension == ".ogg" || extension == ".rcm"
        || extension == ".se" || extension == ".sge") {
        return "resource_path";
    }
    return "resource_stem";
}

std::vector<std::string> parseStringList(std::string value)
{
    std::vector<std::string> result;
    value = trim(value);
    if (value.size() < 2 || value.front() != '[' || value.back() != ']') {
        return result;
    }
    value = value.substr(1, value.size() - 2);

    const std::regex itemRegex(R"('([^']*)'|None)");
    for (std::sregex_iterator it(value.begin(), value.end(), itemRegex), end; it != end; ++it) {
        result.push_back(stripQuotes((*it)[0].str()));
    }
    return result;
}

void skipSpaces(const std::string& value, size_t& pos)
{
    while (pos < value.size() && std::isspace(static_cast<unsigned char>(value[pos]))) {
        ++pos;
    }
}

bool parseIntAt(const std::string& value, size_t& pos, int& out)
{
    skipSpaces(value, pos);
    const size_t start = pos;
    if (pos < value.size() && value[pos] == '-') {
        ++pos;
    }
    const size_t digitsStart = pos;
    while (pos < value.size() && std::isdigit(static_cast<unsigned char>(value[pos]))) {
        ++pos;
    }
    if (digitsStart == pos) {
        pos = start;
        return false;
    }
    out = std::stoi(value.substr(start, pos - start));
    return true;
}

std::string parseCommentValue(const std::string& comment, size_t& pos)
{
    skipSpaces(comment, pos);
    if (pos >= comment.size()) {
        return {};
    }
    if (comment[pos] == '\'') {
        ++pos;
        const size_t start = pos;
        while (pos < comment.size() && comment[pos] != '\'') {
            ++pos;
        }
        std::string value = comment.substr(start, pos - start);
        if (pos < comment.size()) {
            ++pos;
        }
        return value;
    }

    const size_t start = pos;
    while (pos < comment.size() && comment[pos] != ';') {
        ++pos;
    }
    return stripQuotes(trim(comment.substr(start, pos - start)));
}

std::map<std::string, int> parseArgs(const std::string& operands)
{
    std::map<std::string, int> args;
    size_t pos = 0;
    while (pos < operands.size()) {
        if (operands[pos] != 'a' || pos + 2 >= operands.size()
            || !std::isdigit(static_cast<unsigned char>(operands[pos + 1]))) {
            ++pos;
            continue;
        }
        const char slot = operands[pos + 1];
        if (slot < '0' || slot > '3') {
            ++pos;
            continue;
        }
        size_t equals = pos + 2;
        skipSpaces(operands, equals);
        if (equals >= operands.size() || operands[equals] != '=') {
            ++pos;
            continue;
        }
        size_t valuePos = equals + 1;
        int value = 0;
        if (parseIntAt(operands, valuePos, value)) {
            args["a" + std::string(1, slot)] = value;
            pos = valuePos;
            continue;
        }
        ++pos;
    }
    return args;
}

void parseCommentRefs(SqasmInstruction& instruction)
{
    const std::string& comment = instruction.comment;
    if (comment.empty()) {
        return;
    }

    size_t pos = 0;
    while ((pos = comment.find("literal[", pos)) != std::string::npos) {
        size_t indexPos = pos + 8;
        int index = 0;
        if (!parseIntAt(comment, indexPos, index) || indexPos >= comment.size() || comment[indexPos] != ']') {
            ++pos;
            continue;
        }
        size_t valuePos = indexPos + 1;
        skipSpaces(comment, valuePos);
        if (valuePos >= comment.size() || comment[valuePos] != '=') {
            ++pos;
            continue;
        }
        ++valuePos;
        SqasmLiteralRef ref;
        ref.slot = "literal[" + std::to_string(index) + "]";
        ref.index = index;
        ref.value = parseCommentValue(comment, valuePos);
        instruction.literalRefs.push_back(std::move(ref));
        pos = valuePos;
    }

    pos = 0;
    while ((pos = comment.find("literal", pos)) != std::string::npos) {
        size_t indexPos = pos + 7;
        if (indexPos < comment.size() && comment[indexPos] == '[') {
            ++pos;
            continue;
        }
        int index = 0;
        if (!parseIntAt(comment, indexPos, index)) {
            ++pos;
            continue;
        }
        skipSpaces(comment, indexPos);
        if (indexPos >= comment.size() || comment[indexPos] != '=') {
            ++pos;
            continue;
        }
        size_t valuePos = indexPos + 1;
        SqasmLiteralRef ref;
        ref.slot = "literal" + std::to_string(index);
        ref.index = index;
        ref.value = parseCommentValue(comment, valuePos);
        instruction.literalRefs.push_back(std::move(ref));
        pos = valuePos;
    }

    pos = 0;
    while ((pos = comment.find("function[", pos)) != std::string::npos) {
        size_t indexPos = pos + 9;
        int index = 0;
        if (!parseIntAt(comment, indexPos, index) || indexPos >= comment.size() || comment[indexPos] != ']') {
            ++pos;
            continue;
        }
        size_t valuePos = indexPos + 1;
        skipSpaces(comment, valuePos);
        if (valuePos >= comment.size() || comment[valuePos] != '=') {
            ++pos;
            continue;
        }
        ++valuePos;
        SqasmFunctionRef ref;
        ref.index = index;
        ref.value = parseCommentValue(comment, valuePos);
        instruction.functionRefs.push_back(std::move(ref));
        pos = valuePos;
    }

    pos = comment.find("target=");
    if (pos != std::string::npos) {
        pos += 7;
        int target = -1;
        if (parseIntAt(comment, pos, target)) {
            instruction.target = target;
        }
    }
}

bool tryParseInstruction(const std::string& line, SqasmInstruction& instruction)
{
    size_t pos = 0;
    int pc = -1;
    if (!parseIntAt(line, pos, pc)) {
        return false;
    }
    skipSpaces(line, pos);
    if (pos >= line.size() || line[pos] != 'L') {
        return false;
    }
    ++pos;
    int sourceLine = -1;
    if (!parseIntAt(line, pos, sourceLine)) {
        return false;
    }
    skipSpaces(line, pos);
    const size_t opStart = pos;
    while (pos < line.size() && !std::isspace(static_cast<unsigned char>(line[pos]))) {
        ++pos;
    }
    if (opStart == pos) {
        return false;
    }

    const std::string op = line.substr(opStart, pos - opStart);
    std::string operands;
    std::string comment;
    if (pos < line.size()) {
        const std::string rest = line.substr(pos);
        const size_t commentPos = rest.find(';');
        if (commentPos == std::string::npos) {
            operands = rest;
        } else {
            operands = rest.substr(0, commentPos);
            comment = rest.substr(commentPos + 1);
        }
    }

    instruction.pc = pc;
    instruction.sourceLine = sourceLine;
    instruction.op = op;
    instruction.args = parseArgs(operands);
    instruction.comment = trim(comment);
    parseCommentRefs(instruction);
    return true;
}

std::vector<SqasmCall> extractCalls(const std::vector<SqasmInstruction>& instructions)
{
    std::vector<SqasmCall> calls;
    for (const auto& instruction : instructions) {
        if (instruction.op.rfind("_OP_PREPCALL", 0) != 0 || instruction.literalRefs.empty()) {
            continue;
        }
        const auto& first = instruction.literalRefs.front();
        if (first.value.empty()) {
            continue;
        }
        SqasmCall call;
        call.name = first.value;
        call.literalIndex = first.index;
        call.sourceLine = instruction.sourceLine;
        call.pc = instruction.pc;
        calls.push_back(std::move(call));
    }
    return calls;
}

std::vector<SqasmResourceRef> extractResourceRefs(std::vector<SqasmLiteral>& literals)
{
    std::vector<SqasmResourceRef> refs;
    for (auto& literal : literals) {
        std::string kind = resourceKind(literal.value);
        if (kind.empty()) {
            continue;
        }
        literal.kind = kind;
        refs.push_back({literal.index, literal.value, kind});
    }
    return refs;
}

std::vector<SqasmClosureBinding> extractClosureBindings(const std::vector<SqasmInstruction>& instructions)
{
    std::vector<SqasmClosureBinding> bindings;
    SqasmLiteralRef previousLiteral;
    bool hasPreviousLiteral = false;

    for (const auto& instruction : instructions) {
        if (!instruction.literalRefs.empty()) {
            previousLiteral = instruction.literalRefs.front();
            hasPreviousLiteral = true;
        }
        if (instruction.op != "_OP_CLOSURE" || instruction.functionRefs.empty()) {
            continue;
        }
        const auto& ref = instruction.functionRefs.front();
        SqasmClosureBinding binding;
        binding.pc = instruction.pc;
        binding.sourceLine = instruction.sourceLine;
        binding.slot = hasPreviousLiteral ? previousLiteral.value : std::string();
        binding.functionOrdinal = ref.index;
        binding.functionName = ref.value;
        bindings.push_back(std::move(binding));
    }
    return bindings;
}

std::vector<std::string> extractStateCandidates(const SqasmFunction& function)
{
    std::set<std::string> candidates;
    const std::regex stateNameRegex(R"(^(initState|state)[0-9]+$)");
    if (std::regex_match(function.name, stateNameRegex)) {
        candidates.insert(function.name);
    }
    for (const auto& literal : function.literals) {
        const std::string& value = literal.value;
        if (value.rfind("STATE_", 0) == 0 || value.rfind("TITLE_MENU_", 0) == 0 || value.rfind("SELECT_", 0) == 0) {
            candidates.insert(value);
        }
        if (value == "_state" || value == "_nextState" || value == "_scene" || value == "sceneChangeInit") {
            candidates.insert(value);
        }
    }
    return {candidates.begin(), candidates.end()};
}

void finalizeFunction(SqasmFunction& function, std::set<std::string>& moduleResourceRefs, int& moduleCallCount)
{
    function.instructionCount = static_cast<int>(function.instructions.size());
    function.calls = extractCalls(function.instructions);
    function.resourceRefs = extractResourceRefs(function.literals);
    function.closureBindings = extractClosureBindings(function.instructions);
    function.stateCandidates = extractStateCandidates(function);
    moduleCallCount += static_cast<int>(function.calls.size());
    for (const auto& resource : function.resourceRefs) {
        moduleResourceRefs.insert(resource.value);
    }
}

void writeStringArray(std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        out << (i ? ", " : "") << "\"" << core::jsonEscape(values[i]) << "\"";
    }
    out << "]";
}

int closureBindingCount(const SqasmModule& module)
{
    int count = 0;
    for (const auto& function : module.functions) {
        count += static_cast<int>(function.closureBindings.size());
    }
    return count;
}

int literalCount(const SqasmModule& module)
{
    int count = 0;
    for (const auto& function : module.functions) {
        count += static_cast<int>(function.literals.size());
    }
    return count;
}

int localCount(const SqasmModule& module)
{
    int count = 0;
    for (const auto& function : module.functions) {
        count += static_cast<int>(function.locals.size());
    }
    return count;
}

std::vector<std::string> uniqueCallNames(const SqasmModule& module)
{
    std::set<std::string> names;
    for (const auto& function : module.functions) {
        for (const auto& call : function.calls) {
            names.insert(call.name);
        }
    }
    return {names.begin(), names.end()};
}

std::vector<std::string> uniqueStateCandidates(const SqasmModule& module)
{
    std::set<std::string> candidates;
    for (const auto& function : module.functions) {
        for (const auto& candidate : function.stateCandidates) {
            candidates.insert(candidate);
        }
    }
    return {candidates.begin(), candidates.end()};
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
    std::string section;

    const std::regex functionRegex(R"(^\s*function\s+#([0-9]+)\s+('[^']*'|None)\s+source=('[^']*'|\S+)\s+offset=(0x[0-9a-fA-F]+))");
    const std::regex stackRegex(R"(^\s*stack=([0-9]+)\s+generator=(True|False|true|false)\s+varargs=(True|False|true|false))");
    const std::regex parametersRegex(R"(^\s*parameters=(.+)$)");
    const std::regex literalLineRegex(R"(^\s*\[([0-9]+)\]\s+(.+)$)");
    const std::regex localRegex(R"(^\s*(r[0-9]+)\s+('[^']*'|None)\s+ops=(.*)$)");
    const std::regex inputRegex(R"(^;\s*input:\s*(.+)$)");

    std::string line;
    while (std::getline(file, line)) {
        const std::string stripped = trim(line);
        std::smatch match;
        if (std::regex_search(line, match, inputRegex)) {
            module.input = trim(match[1].str());
            continue;
        }
        if (std::regex_search(line, match, functionRegex)) {
            if (current) {
                finalizeFunction(*current, resourceRefs, module.callCount);
            }
            SqasmFunction function;
            function.ordinal = std::stoi(match[1].str());
            function.name = stripQuotes(match[2].str());
            function.source = stripQuotes(match[3].str());
            function.offset = match[4].str();
            module.functions.push_back(std::move(function));
            current = &module.functions.back();
            section.clear();
            continue;
        }
        if (!current) {
            continue;
        }

        if (std::regex_search(line, match, stackRegex)) {
            current->stack = std::stoi(match[1].str());
            current->generator = parseBool(match[2].str());
            current->varargs = parseBool(match[3].str());
            continue;
        }
        if (std::regex_search(line, match, parametersRegex)) {
            current->parameters = parseStringList(match[1].str());
            continue;
        }
        if (stripped == "literals:") {
            section = "literals";
            continue;
        }
        if (stripped == "locals:") {
            section = "locals";
            continue;
        }
        if (stripped == "instructions:") {
            section = "instructions";
            continue;
        }

        if (section == "literals" && std::regex_search(line, match, literalLineRegex)) {
            SqasmLiteral literal;
            literal.index = std::stoi(match[1].str());
            literal.raw = trim(match[2].str());
            literal.value = stripQuotes(literal.raw);
            current->literals.push_back(std::move(literal));
            continue;
        }
        if (section == "locals" && std::regex_search(line, match, localRegex)) {
            SqasmLocal local;
            local.reg = match[1].str();
            local.name = stripQuotes(match[2].str());
            local.ops = trim(match[3].str());
            current->locals.push_back(std::move(local));
            continue;
        }
        if (section == "instructions") {
            SqasmInstruction instruction;
            if (tryParseInstruction(line, instruction)) {
                ++module.instructionCount;
                current->instructions.push_back(std::move(instruction));
            }
        }
    }
    if (current) {
        finalizeFunction(*current, resourceRefs, module.callCount);
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
                std::error_code error;
                const auto resolved = std::filesystem::weakly_canonical(candidate, error);
                return error ? std::filesystem::absolute(candidate) : resolved;
            }
        }
    }
    return {};
}

std::string sqasmModuleReportToJson(const SqasmModule& module)
{
    std::ostringstream out;
    const auto calls = uniqueCallNames(module);
    const auto stateCandidates = uniqueStateCandidates(module);

    out << "{\n";
    out << "  \"schema\": \"yuengine.sqasm_module_report.v1\",\n";
    out << "  \"path\": \"" << core::jsonEscape(module.path.string()) << "\",\n";
    out << "  \"input\": \"" << core::jsonEscape(module.input) << "\",\n";
    out << "  \"metrics\": \"functions=" << module.functions.size() << " instructions=" << module.instructionCount
        << " calls=" << module.callCount << " resources=" << module.resourceRefs.size() << " closure_bindings="
        << closureBindingCount(module) << "\",\n";
    out << "  \"functions\": " << module.functions.size() << ",\n";
    out << "  \"instructions\": " << module.instructionCount << ",\n";
    out << "  \"literals\": " << literalCount(module) << ",\n";
    out << "  \"locals\": " << localCount(module) << ",\n";
    out << "  \"calls\": " << module.callCount << ",\n";
    out << "  \"unique_calls\": " << calls.size() << ",\n";
    out << "  \"resource_refs\": " << module.resourceRefs.size() << ",\n";
    out << "  \"closure_bindings\": " << closureBindingCount(module) << ",\n";
    out << "  \"state_candidate_count\": " << stateCandidates.size() << ",\n";
    out << "  \"resources\": ";
    writeStringArray(out, module.resourceRefs);
    out << ",\n";
    out << "  \"state_candidates\": ";
    writeStringArray(out, stateCandidates);
    out << ",\n";
    out << "  \"call_names\": ";
    writeStringArray(out, calls);
    out << ",\n";
    out << "  \"function_summaries\": [\n";
    for (size_t i = 0; i < module.functions.size(); ++i) {
        const auto& function = module.functions[i];
        out << "    {";
        out << "\"ordinal\": " << function.ordinal;
        out << ", \"name\": \"" << core::jsonEscape(function.name) << "\"";
        out << ", \"source\": \"" << core::jsonEscape(function.source) << "\"";
        out << ", \"offset\": \"" << core::jsonEscape(function.offset) << "\"";
        out << ", \"stack\": " << function.stack;
        out << ", \"generator\": " << (function.generator ? "true" : "false");
        out << ", \"varargs\": " << (function.varargs ? "true" : "false");
        out << ", \"parameters\": ";
        writeStringArray(out, function.parameters);
        out << ", \"literals\": " << function.literals.size();
        out << ", \"locals\": " << function.locals.size();
        out << ", \"instructions\": " << function.instructionCount;
        out << ", \"calls\": " << function.calls.size();
        out << ", \"resource_refs\": " << function.resourceRefs.size();
        out << ", \"closure_bindings\": " << function.closureBindings.size();
        out << ", \"state_candidates\": ";
        writeStringArray(out, function.stateCandidates);
        out << "}";
        out << (i + 1 == module.functions.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

} // namespace yu::script
