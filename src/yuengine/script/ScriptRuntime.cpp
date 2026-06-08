#include "yuengine/script/ScriptRuntime.h"

#include "yuengine/core/Json.h"

#include <map>
#include <set>
#include <sstream>
#include <utility>

namespace yu::script {
namespace {

std::map<std::string, std::vector<int>> buildFunctionNameIndex(const SqasmModule& module)
{
    std::map<std::string, std::vector<int>> index;
    for (const auto& function : module.functions) {
        index[function.name].push_back(function.ordinal);
    }
    return index;
}

const SqasmFunction* findUniqueEntryFunction(const SqasmModule& module, const std::string& name)
{
    const SqasmFunction* found = nullptr;
    for (const auto& function : module.functions) {
        if (function.name != name) {
            continue;
        }
        if (found) {
            return nullptr;
        }
        found = &function;
    }
    return found;
}

const SqasmFunction* findRootFunction(const SqasmModule& module)
{
    for (const auto& function : module.functions) {
        if (function.ordinal == 0) {
            return &function;
        }
    }
    return module.functions.empty() ? nullptr : &module.functions.front();
}

const SqasmFunction* findFunctionByOrdinal(const SqasmModule& module, int ordinal)
{
    for (const auto& function : module.functions) {
        if (function.ordinal == ordinal) {
            return &function;
        }
    }
    return nullptr;
}

int argValue(const SqasmInstruction& instruction, const std::string& key, int fallback = -1)
{
    const auto found = instruction.args.find(key);
    return found == instruction.args.end() ? fallback : found->second;
}

std::string literalValueByIndex(const SqasmFunction& function, int index)
{
    for (const auto& literal : function.literals) {
        if (literal.index == index) {
            return literal.value;
        }
    }
    return {};
}

std::string classNameBefore(const std::vector<SqasmInstruction>& instructions, size_t classInstruction)
{
    const size_t begin = classInstruction > 5 ? classInstruction - 5 : 0;
    for (size_t i = classInstruction; i-- > begin;) {
        const auto& instruction = instructions[i];
        if (instruction.literalRefs.empty()) {
            continue;
        }
        if (instruction.op == "_OP_DLOAD" || instruction.op == "_OP_LOAD") {
            return instruction.literalRefs.front().value;
        }
    }
    return {};
}

std::map<int, int> buildRootFunctionRefOrdinalMap(const SqasmModule& module)
{
    const SqasmFunction* root = findRootFunction(module);
    if (!root) {
        return {};
    }

    std::map<int, int> byRefIndex;
    size_t functionCursor = 0;
    while (functionCursor < module.functions.size() && module.functions[functionCursor].ordinal == 0) {
        ++functionCursor;
    }

    for (const auto& binding : root->closureBindings) {
        for (; functionCursor < module.functions.size(); ++functionCursor) {
            const auto& function = module.functions[functionCursor];
            if (function.name != binding.functionName) {
                continue;
            }
            byRefIndex[binding.functionOrdinal] = function.ordinal;
            ++functionCursor;
            break;
        }
    }
    return byRefIndex;
}

int resolveFunctionRefOrdinal(
    const SqasmModule& module,
    const std::map<int, int>& rootFunctionRefOrdinals,
    int functionRefIndex,
    const std::string& functionName)
{
    const auto mapped = rootFunctionRefOrdinals.find(functionRefIndex);
    if (mapped != rootFunctionRefOrdinals.end()) {
        const SqasmFunction* function = findFunctionByOrdinal(module, mapped->second);
        if (function && function->name == functionName) {
            return function->ordinal;
        }
    }

    const SqasmFunction* bySquirrelRef = findFunctionByOrdinal(module, functionRefIndex + 1);
    if (bySquirrelRef && bySquirrelRef->name == functionName) {
        return bySquirrelRef->ordinal;
    }

    const SqasmFunction* byExactOrdinal = findFunctionByOrdinal(module, functionRefIndex);
    if (byExactOrdinal && byExactOrdinal->name == functionName) {
        return byExactOrdinal->ordinal;
    }

    int uniqueOrdinal = -1;
    for (const auto& function : module.functions) {
        if (function.name != functionName) {
            continue;
        }
        if (uniqueOrdinal != -1) {
            return -1;
        }
        uniqueOrdinal = function.ordinal;
    }
    return uniqueOrdinal;
}

std::vector<ScriptMethodBinding> buildMethodBindings(const SqasmModule& module)
{
    const SqasmFunction* root = findRootFunction(module);
    if (!root) {
        return {};
    }

    const auto rootFunctionRefOrdinals = buildRootFunctionRefOrdinalMap(module);
    std::vector<ScriptMethodBinding> bindings;
    const auto& instructions = root->instructions;
    for (size_t i = 0; i < instructions.size(); ++i) {
        const auto& instruction = instructions[i];
        if (instruction.op != "_OP_CLASS") {
            continue;
        }

        const std::string ownerClass = classNameBefore(instructions, i);
        if (ownerClass.empty()) {
            continue;
        }

        std::string currentSlot;
        for (size_t j = i + 1; j < instructions.size(); ++j) {
            const auto& current = instructions[j];
            if (current.op == "_OP_CLASS") {
                break;
            }
            if (current.op == "_OP_NEWSLOT") {
                break;
            }
            if (!current.literalRefs.empty()) {
                currentSlot = current.literalRefs.front().value;
            }
            if (current.op != "_OP_CLOSURE" || current.functionRefs.empty()) {
                continue;
            }

            const auto& ref = current.functionRefs.front();
            ScriptMethodBinding binding;
            binding.ownerClass = ownerClass;
            binding.classPc = instruction.pc;
            binding.sourceLine = instruction.sourceLine;
            binding.slot = currentSlot.empty() ? ref.value : currentSlot;
            binding.functionRefIndex = ref.index;
            binding.functionOrdinal =
                resolveFunctionRefOrdinal(module, rootFunctionRefOrdinals, ref.index, ref.value);
            binding.functionName = ref.value;
            bindings.push_back(std::move(binding));
        }
    }

    return bindings;
}

std::set<std::string> classNamesFromMethodBindings(const std::vector<ScriptMethodBinding>& bindings)
{
    std::set<std::string> names;
    for (const auto& binding : bindings) {
        names.insert(binding.ownerClass);
    }
    return names;
}

std::vector<ScriptObjectBinding> buildObjectBindings(
    const SqasmModule& module,
    const std::vector<ScriptMethodBinding>& methodBindings)
{
    const SqasmFunction* root = findRootFunction(module);
    if (!root) {
        return {};
    }

    const auto classNames = classNamesFromMethodBindings(methodBindings);
    std::vector<ScriptObjectBinding> bindings;
    const auto& instructions = root->instructions;
    for (size_t i = 0; i + 3 < instructions.size(); ++i) {
        const auto& instruction = instructions[i];
        if (instruction.op != "_OP_DLOAD" || instruction.literalRefs.empty()) {
            continue;
        }
        if (instructions[i + 1].op != "_OP_PREPCALL" || instructions[i + 2].op != "_OP_CALL"
            || instructions[i + 3].op != "_OP_NEWSLOT") {
            continue;
        }

        const std::string objectName = instruction.literalRefs.front().value;
        const std::string className = literalValueByIndex(*root, argValue(instruction, "a3"));
        if (objectName.empty() || className.empty() || classNames.find(className) == classNames.end()) {
            continue;
        }

        ScriptObjectBinding binding;
        binding.objectName = objectName;
        binding.className = className;
        binding.pc = instruction.pc;
        binding.sourceLine = instruction.sourceLine;
        binding.evidence = "_OP_DLOAD/_OP_PREPCALL/_OP_CALL/_OP_NEWSLOT";
        bindings.push_back(std::move(binding));
    }
    return bindings;
}

const ScriptObjectBinding* findObjectBinding(
    const std::vector<ScriptObjectBinding>& objectBindings,
    const std::string& objectName)
{
    for (const auto& binding : objectBindings) {
        if (binding.objectName == objectName) {
            return &binding;
        }
    }
    return nullptr;
}

const ScriptMethodBinding* findMethodBinding(
    const std::vector<ScriptMethodBinding>& methodBindings,
    const std::string& ownerClass,
    const std::string& slot)
{
    for (const auto& binding : methodBindings) {
        if (binding.ownerClass == ownerClass && binding.slot == slot) {
            return &binding;
        }
    }
    return nullptr;
}

const SqasmInstruction* findInstructionByPc(const SqasmFunction& function, int pc)
{
    for (const auto& instruction : function.instructions) {
        if (instruction.pc == pc) {
            return &instruction;
        }
    }
    return nullptr;
}

std::string receiverForCall(const SqasmFunction& function, const SqasmCall& call)
{
    const SqasmInstruction* callInstruction = findInstructionByPc(function, call.pc);
    if (!callInstruction) {
        return {};
    }

    const int receiverReg = argValue(*callInstruction, "a2");
    if (receiverReg <= 0) {
        return {};
    }

    for (auto it = function.instructions.rbegin(); it != function.instructions.rend(); ++it) {
        const auto& instruction = *it;
        if (instruction.pc >= call.pc) {
            continue;
        }
        if (call.pc - instruction.pc > 8) {
            break;
        }
        if (argValue(instruction, "a0") != receiverReg) {
            continue;
        }
        if ((instruction.op == "_OP_GETK" || instruction.op == "_OP_DLOAD") && !instruction.literalRefs.empty()) {
            return instruction.literalRefs.front().value;
        }
    }
    return {};
}

bool isBuiltinCall(const std::string& name)
{
    static const std::set<std::string> builtins = {"print"};
    return builtins.find(name) != builtins.end();
}

void writeOrdinalArray(std::ostringstream& out, const std::vector<int>& values)
{
    out << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        out << (i ? ", " : "") << values[i];
    }
    out << "]";
}

void writeMethodBindings(std::ostringstream& out, const std::vector<ScriptMethodBinding>& bindings)
{
    std::map<std::string, std::vector<const ScriptMethodBinding*>> byClass;
    for (const auto& binding : bindings) {
        byClass[binding.ownerClass].push_back(&binding);
    }

    out << "[\n";
    size_t classIndex = 0;
    for (const auto& [ownerClass, methods] : byClass) {
        out << "    {\"class\": \"" << core::jsonEscape(ownerClass) << "\", \"method_count\": " << methods.size()
            << ", \"methods\": [";
        for (size_t i = 0; i < methods.size(); ++i) {
            const auto& method = *methods[i];
            out << "{\"slot\": \"" << core::jsonEscape(method.slot) << "\", \"function_name\": \""
                << core::jsonEscape(method.functionName) << "\", \"function_ref_index\": " << method.functionRefIndex
                << ", \"function_ordinal\": " << method.functionOrdinal << ", \"source_line\": "
                << method.sourceLine << "}";
            out << (i + 1 == methods.size() ? "" : ", ");
        }
        out << "]}";
        out << (++classIndex == byClass.size() ? "\n" : ",\n");
    }
    out << "  ]";
}

void writeObjectBindings(std::ostringstream& out, const std::vector<ScriptObjectBinding>& bindings)
{
    out << "[\n";
    for (size_t i = 0; i < bindings.size(); ++i) {
        const auto& binding = bindings[i];
        out << "    {\"object\": \"" << core::jsonEscape(binding.objectName) << "\", \"class\": \""
            << core::jsonEscape(binding.className) << "\", \"pc\": " << binding.pc << ", \"source_line\": "
            << binding.sourceLine << ", \"evidence\": \"" << core::jsonEscape(binding.evidence) << "\"}";
        out << (i + 1 == bindings.size() ? "\n" : ",\n");
    }
    out << "  ]";
}

} // namespace

ScriptExecutionPlan planEntryExecution(
    const SqasmModule& module,
    const std::string& entryFunction,
    const native::NativeRegistry& registry)
{
    ScriptExecutionPlan plan;
    plan.modulePath = module.path.string();
    plan.entryFunction = entryFunction;
    plan.methodBindings = buildMethodBindings(module);
    plan.objectBindings = buildObjectBindings(module, plan.methodBindings);
    plan.classMethodTableCount = static_cast<int>(classNamesFromMethodBindings(plan.methodBindings).size());
    plan.methodBindingCount = static_cast<int>(plan.methodBindings.size());
    plan.objectBindingCount = static_cast<int>(plan.objectBindings.size());

    const auto functionIndex = buildFunctionNameIndex(module);
    const SqasmFunction* entry = findUniqueEntryFunction(module, entryFunction);
    if (!entry) {
        plan.status = "entry_not_found_or_ambiguous";
        return plan;
    }

    plan.entryFound = true;
    plan.entryOrdinal = entry->ordinal;
    plan.entryInstructions = entry->instructionCount;
    plan.status = "vm_not_implemented";
    plan.directCalls = static_cast<int>(entry->calls.size());

    for (const auto& call : entry->calls) {
        ScriptCallResolution resolution;
        resolution.name = call.name;
        resolution.pc = call.pc;
        resolution.sourceLine = call.sourceLine;
        resolution.receiver = receiverForCall(*entry, call);

        if (!resolution.receiver.empty()) {
            const ScriptObjectBinding* object = findObjectBinding(plan.objectBindings, resolution.receiver);
            if (object) {
                resolution.ownerClass = object->className;
                const ScriptMethodBinding* method =
                    findMethodBinding(plan.methodBindings, object->className, call.name);
                if (method && method->functionOrdinal >= 0) {
                    resolution.category = "script_object_method";
                    resolution.evidence = object->evidence + " + receiver _OP_GETK";
                    resolution.candidateOrdinals.push_back(method->functionOrdinal);
                    ++plan.objectMethodCalls;
                    plan.callResolutions.push_back(std::move(resolution));
                    continue;
                }
            }
        }

        const native::ApiSurface* api = registry.find(call.name);
        if (api) {
            resolution.category = "native_obligation";
            resolution.service = api->service;
            resolution.implementationStatus =
                api->implementationStatus.empty() ? "not_started" : api->implementationStatus;
            ++plan.nativeObligations;
            plan.callResolutions.push_back(std::move(resolution));
            continue;
        }

        if (isBuiltinCall(call.name)) {
            resolution.category = "builtin";
            resolution.evidence = "squirrel builtin";
            ++plan.builtinCalls;
            plan.callResolutions.push_back(std::move(resolution));
            continue;
        }

        const auto candidates = functionIndex.find(call.name);
        if (candidates == functionIndex.end()) {
            resolution.category = "unresolved_or_builtin";
            ++plan.unresolvedCalls;
            plan.callResolutions.push_back(std::move(resolution));
            continue;
        }

        resolution.candidateOrdinals = candidates->second;
        if (candidates->second.size() == 1) {
            resolution.category = "script_function";
            ++plan.scriptCalls;
        } else {
            resolution.category = "ambiguous_script_function";
            ++plan.ambiguousCalls;
        }
        plan.callResolutions.push_back(std::move(resolution));
    }

    return plan;
}

std::string scriptExecutionPlanToJson(const ScriptExecutionPlan& plan)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.script_execution_plan.v1\",\n";
    out << "  \"module\": \"" << core::jsonEscape(plan.modulePath) << "\",\n";
    out << "  \"entry_function\": \"" << core::jsonEscape(plan.entryFunction) << "\",\n";
    out << "  \"metrics\": \"entry=" << core::jsonEscape(plan.entryFunction)
        << " found=" << (plan.entryFound ? "true" : "false") << " direct_calls=" << plan.directCalls
        << " builtin_calls=" << plan.builtinCalls << " native_obligations=" << plan.nativeObligations
        << " object_method_calls=" << plan.objectMethodCalls << " script_calls=" << plan.scriptCalls
        << " ambiguous_calls=" << plan.ambiguousCalls << " unresolved_calls=" << plan.unresolvedCalls
        << " class_method_tables=" << plan.classMethodTableCount << " object_bindings=" << plan.objectBindingCount
        << " status=" << core::jsonEscape(plan.status) << "\",\n";
    out << "  \"entry_found\": " << (plan.entryFound ? "true" : "false") << ",\n";
    out << "  \"entry_ordinal\": " << plan.entryOrdinal << ",\n";
    out << "  \"entry_instructions\": " << plan.entryInstructions << ",\n";
    out << "  \"status\": \"" << core::jsonEscape(plan.status) << "\",\n";
    out << "  \"direct_calls\": " << plan.directCalls << ",\n";
    out << "  \"builtin_calls\": " << plan.builtinCalls << ",\n";
    out << "  \"native_obligations\": " << plan.nativeObligations << ",\n";
    out << "  \"object_method_calls\": " << plan.objectMethodCalls << ",\n";
    out << "  \"script_calls\": " << plan.scriptCalls << ",\n";
    out << "  \"ambiguous_calls\": " << plan.ambiguousCalls << ",\n";
    out << "  \"unresolved_calls\": " << plan.unresolvedCalls << ",\n";
    out << "  \"class_method_tables\": " << plan.classMethodTableCount << ",\n";
    out << "  \"method_bindings\": " << plan.methodBindingCount << ",\n";
    out << "  \"object_bindings\": " << plan.objectBindingCount << ",\n";
    out << "  \"class_method_table_details\": ";
    writeMethodBindings(out, plan.methodBindings);
    out << ",\n";
    out << "  \"object_binding_details\": ";
    writeObjectBindings(out, plan.objectBindings);
    out << ",\n";
    out << "  \"call_resolutions\": [\n";
    for (size_t i = 0; i < plan.callResolutions.size(); ++i) {
        const auto& resolution = plan.callResolutions[i];
        out << "    {\"name\": \"" << core::jsonEscape(resolution.name) << "\", \"pc\": " << resolution.pc
            << ", \"source_line\": " << resolution.sourceLine << ", \"receiver\": \""
            << core::jsonEscape(resolution.receiver) << "\", \"owner_class\": \""
            << core::jsonEscape(resolution.ownerClass) << "\", \"category\": \""
            << core::jsonEscape(resolution.category) << "\", \"service\": \""
            << core::jsonEscape(resolution.service) << "\", \"implementation_status\": \""
            << core::jsonEscape(resolution.implementationStatus) << "\", \"evidence\": \""
            << core::jsonEscape(resolution.evidence) << "\", \"candidate_ordinals\": ";
        writeOrdinalArray(out, resolution.candidateOrdinals);
        out << "}";
        out << (i + 1 == plan.callResolutions.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

} // namespace yu::script
