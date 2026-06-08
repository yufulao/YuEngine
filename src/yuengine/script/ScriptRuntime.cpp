#include "yuengine/script/ScriptRuntime.h"

#include "yuengine/core/Json.h"

#include <map>
#include <set>
#include <sstream>
#include <utility>

namespace yu::script {
namespace {

struct FunctionSlotBinding {
    std::string slot;
    int functionRefIndex = -1;
    int functionOrdinal = -1;
    std::string functionName;
    int pc = -1;
    int sourceLine = -1;
    std::string evidence;
};

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

std::vector<FunctionSlotBinding> buildRootFunctionSlots(const SqasmModule& module)
{
    const SqasmFunction* root = findRootFunction(module);
    if (!root) {
        return {};
    }

    const auto rootFunctionRefOrdinals = buildRootFunctionRefOrdinalMap(module);
    std::vector<FunctionSlotBinding> bindings;
    const auto& instructions = root->instructions;
    for (size_t i = 0; i + 2 < instructions.size(); ++i) {
        const auto& loadSlot = instructions[i];
        const auto& closure = instructions[i + 1];
        const auto& newSlot = instructions[i + 2];
        if (loadSlot.op != "_OP_LOAD" || loadSlot.literalRefs.empty()) {
            continue;
        }
        if (closure.op != "_OP_CLOSURE" || closure.functionRefs.empty() || newSlot.op != "_OP_NEWSLOT") {
            continue;
        }

        const auto& ref = closure.functionRefs.front();
        FunctionSlotBinding binding;
        binding.slot = loadSlot.literalRefs.front().value;
        binding.functionRefIndex = ref.index;
        binding.functionOrdinal =
            resolveFunctionRefOrdinal(module, rootFunctionRefOrdinals, ref.index, ref.value);
        binding.functionName = ref.value;
        binding.pc = loadSlot.pc;
        binding.sourceLine = loadSlot.sourceLine;
        binding.evidence = "root _OP_LOAD/_OP_CLOSURE/_OP_NEWSLOT";
        bindings.push_back(std::move(binding));
    }
    return bindings;
}

std::vector<ScriptObjectMethodSlot> buildRootObjectMethodSlots(const SqasmModule& module)
{
    const SqasmFunction* root = findRootFunction(module);
    if (!root) {
        return {};
    }

    const auto rootFunctionRefOrdinals = buildRootFunctionRefOrdinalMap(module);
    std::vector<ScriptObjectMethodSlot> bindings;
    const auto& instructions = root->instructions;
    for (size_t i = 0; i + 3 < instructions.size(); ++i) {
        const auto& getObject = instructions[i];
        const auto& loadSlot = instructions[i + 1];
        const auto& closure = instructions[i + 2];
        const auto& newSlot = instructions[i + 3];
        if (getObject.op != "_OP_GETK" || getObject.literalRefs.empty()) {
            continue;
        }
        if (loadSlot.op != "_OP_LOAD" || loadSlot.literalRefs.empty()) {
            continue;
        }
        if (closure.op != "_OP_CLOSURE" || closure.functionRefs.empty() || newSlot.op != "_OP_NEWSLOT") {
            continue;
        }

        const auto& ref = closure.functionRefs.front();
        ScriptObjectMethodSlot binding;
        binding.objectName = getObject.literalRefs.front().value;
        binding.slot = loadSlot.literalRefs.front().value;
        binding.functionRefIndex = ref.index;
        binding.functionOrdinal =
            resolveFunctionRefOrdinal(module, rootFunctionRefOrdinals, ref.index, ref.value);
        binding.functionName = ref.value;
        binding.pc = getObject.pc;
        binding.sourceLine = getObject.sourceLine;
        binding.evidence = "root object _OP_GETK/_OP_LOAD/_OP_CLOSURE/_OP_NEWSLOT";
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

const FunctionSlotBinding* findFunctionSlot(const std::vector<FunctionSlotBinding>& slots, const std::string& slot)
{
    for (const auto& binding : slots) {
        if (binding.slot == slot) {
            return &binding;
        }
    }
    return nullptr;
}

const ScriptObjectMethodSlot* findObjectMethodSlot(
    const std::vector<ScriptObjectMethodSlot>& slots,
    const std::string& objectName,
    const std::string& slot)
{
    for (const auto& binding : slots) {
        if (binding.objectName == objectName && binding.slot == slot) {
            return &binding;
        }
    }
    return nullptr;
}

bool hasClass(const std::vector<ScriptMethodBinding>& methodBindings, const std::string& className)
{
    for (const auto& binding : methodBindings) {
        if (binding.ownerClass == className) {
            return true;
        }
    }
    return false;
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
    static const std::set<std::string> builtins = {"len", "print", "tofloat", "tointeger"};
    return builtins.find(name) != builtins.end();
}

bool isSquirrelValueMethod(const std::string& name)
{
    static const std::set<std::string> methods = {"count", "get", "isActive", "toPoint", "toAbsPos"};
    return methods.find(name) != methods.end();
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

void writeStringArray(std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        out << (i ? ", " : "") << "\"" << core::jsonEscape(values[i]) << "\"";
    }
    out << "]";
}

std::string sceneObjectNameForConstructorCall(
    const SqasmFunction& function,
    const SqasmCall& call,
    const std::string& ownerObject)
{
    for (auto it = function.instructions.rbegin(); it != function.instructions.rend(); ++it) {
        const auto& instruction = *it;
        if (instruction.pc >= call.pc) {
            continue;
        }
        if (call.pc - instruction.pc > 4) {
            break;
        }
        if (instruction.op == "_OP_LOADINT") {
            const int index = argValue(instruction, "a1", -1);
            if (index >= 0) {
                return ownerObject + "._scenes[" + std::to_string(index) + "]";
            }
        }
    }
    return ownerObject + "._constructed." + call.name + "@" + std::to_string(call.pc);
}

class StaticScriptExecutor {
public:
    StaticScriptExecutor(
        const SqasmModule& module,
        const native::NativeRegistry& registry,
        const native::NativeServiceCatalog& catalog)
        : module_(module)
        , registry_(registry)
        , catalog_(catalog)
        , methodBindings_(buildMethodBindings(module))
        , objectBindings_(buildObjectBindings(module, methodBindings_))
        , objectMethodSlots_(buildRootObjectMethodSlots(module))
        , functionSlots_(buildRootFunctionSlots(module))
    {
        for (const auto& binding : objectMethodSlots_) {
            rootObjects_.insert(binding.objectName);
        }
    }

    ScriptExecutionReport run(const std::string& entryFunction, int frames)
    {
        ScriptExecutionReport report;
        report.modulePath = module_.path.string();
        report.entryFunction = entryFunction;
        report.frames = frames < 0 ? 0 : frames;
        report.executionMode = "static_bytecode_call_trace_branch_sensitive_boot_edge";
        report.classMethodTableCount = static_cast<int>(classNamesFromMethodBindings(methodBindings_).size());
        report.methodBindingCount = static_cast<int>(methodBindings_.size());
        report.objectBindingCount = static_cast<int>(objectBindings_.size());
        report.rootObjectMethodSlots = static_cast<int>(objectMethodSlots_.size());
        report.objectMethodSlots = objectMethodSlots_;
        report_ = &report;

        const SqasmFunction* entry = findEntrypoint(entryFunction);
        if (!entry) {
            report.status = "entry_not_found_or_ambiguous";
            report_ = nullptr;
            return report;
        }

        report.entryFound = true;
        bootstrapRootObjects();
        executeFunction(*entry, {}, {}, 0);
        for (int frame = 0; frame < report.frames; ++frame) {
            executeGlobalFunctionSlot("main", 0);
        }

        report.constructedObjects = static_cast<int>(report.constructedObjectDetails.size());
        report.executed = true;
        report.status = report.truncated ? "trace_truncated_not_full_vm" : "trace_ready_not_full_vm";
        report_ = nullptr;
        return report;
    }

private:
    const SqasmFunction* findEntrypoint(const std::string& slot) const
    {
        const FunctionSlotBinding* rootSlot = findFunctionSlot(functionSlots_, slot);
        if (rootSlot && rootSlot->functionOrdinal >= 0) {
            return findFunctionByOrdinal(module_, rootSlot->functionOrdinal);
        }
        return findUniqueEntryFunction(module_, slot);
    }

    void bootstrapRootObjects()
    {
        rootObjects_.insert("gMenu");
        for (const auto& binding : objectBindings_) {
            constructObject(binding.objectName, binding.className, binding.pc, binding.sourceLine, binding.evidence, 0);
        }
    }

    bool recordEvent(ScriptExecutionEvent event)
    {
        if (!report_) {
            return false;
        }
        if (report_->events.size() >= maxEvents_) {
            report_->truncated = true;
            return false;
        }
        report_->events.push_back(std::move(event));
        return true;
    }

    void constructObject(
        const std::string& objectName,
        const std::string& className,
        int pc,
        int sourceLine,
        const std::string& evidence,
        int depth)
    {
        if (objectName.empty() || className.empty()) {
            return;
        }
        if (depth > maxDepth_) {
            if (report_) {
                report_->truncated = true;
            }
            return;
        }

        const auto existing = objectClasses_.find(objectName);
        const bool isNewObject = existing == objectClasses_.end();
        if (isNewObject) {
            objectClasses_[objectName] = className;
            if (report_) {
                report_->constructedObjectDetails.push_back({objectName, className, pc, sourceLine, evidence});
            }
            recordEvent({"construct_object", {}, -1, objectName, className, className, {}, "script_class", {},
                sourceLine, pc, evidence});
        }

        if (isNewObject && objectName.find("._scenes[") != std::string::npos) {
            const std::string owner = objectName.substr(0, objectName.find("._scenes["));
            sceneObjectsByOwner_[owner].push_back(objectName);
        }

        const ScriptMethodBinding* constructor = findMethodBinding(methodBindings_, className, "constructor");
        if (constructor) {
            executeMethod(*constructor, objectName, className, depth + 1, "script_constructor");
        }
    }

    void executeGlobalFunctionSlot(const std::string& slot, int depth)
    {
        const FunctionSlotBinding* binding = findFunctionSlot(functionSlots_, slot);
        if (!binding || binding->functionOrdinal < 0) {
            markUnresolved(slot, {}, {}, -1, -1, "root function slot not recovered");
            return;
        }
        const SqasmFunction* function = findFunctionByOrdinal(module_, binding->functionOrdinal);
        if (!function) {
            markUnresolved(slot, {}, {}, binding->sourceLine, binding->pc, "root function ordinal not found");
            return;
        }
        recordEvent({"execute_global_function", function->name, function->ordinal, {}, {}, slot, {}, "script_function",
            {}, binding->sourceLine, binding->pc, binding->evidence});
        if (report_) {
            ++report_->scriptFunctions;
        }
        executeFunction(*function, {}, {}, depth + 1);
    }

    void executeObjectMethodSlot(
        const ScriptObjectMethodSlot& binding,
        const std::string& receiver,
        const SqasmCall& call,
        int depth)
    {
        const SqasmFunction* function = findFunctionByOrdinal(module_, binding.functionOrdinal);
        if (!function) {
            markUnresolved(call.name, receiver, {}, call.sourceLine, call.pc, "root object method ordinal not found");
            return;
        }
        recordEvent({"execute_object_function", function->name, function->ordinal, receiver, {}, call.name, receiver,
            "script_object_function", {}, call.sourceLine, call.pc, binding.evidence});
        if (report_) {
            ++report_->scriptFunctions;
        }
        executeFunction(*function, receiver, {}, depth + 1);
    }

    void executeMethod(
        const ScriptMethodBinding& binding,
        const std::string& ownerObject,
        const std::string& ownerClass,
        int depth,
        const std::string& category)
    {
        if (depth > maxDepth_) {
            if (report_) {
                report_->truncated = true;
            }
            return;
        }

        const SqasmFunction* function = findFunctionByOrdinal(module_, binding.functionOrdinal);
        if (!function) {
            markUnresolved(binding.slot, ownerObject, ownerClass, binding.sourceLine, binding.classPc,
                "method binding ordinal not found");
            return;
        }

        const std::string activeKey =
            ownerObject + "|" + ownerClass + "|" + binding.slot + "|" + std::to_string(function->ordinal);
        if (activeMethods_.find(activeKey) != activeMethods_.end()) {
            recordEvent({"skip_recursive_method", function->name, function->ordinal, ownerObject, ownerClass,
                binding.slot, ownerObject, "recursion_guard", {}, binding.sourceLine, binding.classPc,
                "already active in call stack"});
            return;
        }

        activeMethods_.insert(activeKey);
        recordEvent({"execute_method", function->name, function->ordinal, ownerObject, ownerClass, binding.slot,
            ownerObject, category, {}, binding.sourceLine, binding.classPc,
            "_OP_CLASS/_OP_CLOSURE/_OP_NEWSLOTA method binding"});
        if (report_) {
            ++report_->scriptMethods;
        }
        executeFunction(*function, ownerObject, ownerClass, depth + 1);
        activeMethods_.erase(activeKey);
    }

    void executeFunction(
        const SqasmFunction& function,
        const std::string& ownerObject,
        const std::string& ownerClass,
        int depth)
    {
        if (depth > maxDepth_) {
            if (report_) {
                report_->truncated = true;
            }
            return;
        }

        bool recordedMainSkip = false;
        for (const auto& call : function.calls) {
            if (shouldSkipForBootState(function, ownerClass, call)) {
                if (!recordedMainSkip) {
                    recordEvent({"skip_boot_state_branch", function.name, function.ordinal, ownerObject, ownerClass,
                        call.name, {}, "control_flow_deferred", {}, call.sourceLine, call.pc,
                        "ModuleTitle._nextState == 300 boot edge; later state branches require value VM"});
                    recordedMainSkip = true;
                }
                continue;
            }
            resolveAndExecuteCall(function, ownerObject, ownerClass, call, depth + 1);
        }
    }

    bool shouldSkipForBootState(
        const SqasmFunction& function,
        const std::string& ownerClass,
        const SqasmCall& call) const
    {
        return ownerClass == "ModuleTitle" && function.name == "main" && call.pc > 13;
    }

    void resolveAndExecuteCall(
        const SqasmFunction& function,
        const std::string& ownerObject,
        const std::string& ownerClass,
        const SqasmCall& call,
        int depth)
    {
        if (isBuiltinCall(call.name)) {
            if (report_) {
                ++report_->builtinCalls;
            }
            recordEvent({"builtin_call", function.name, function.ordinal, ownerObject, ownerClass, call.name, {},
                "builtin", {}, call.sourceLine, call.pc, "squirrel builtin"});
            return;
        }

        const std::string receiver = receiverForCall(function, call);
        if (!receiver.empty()) {
            if (executeReceiverCall(receiver, function, ownerObject, ownerClass, call, depth)) {
                return;
            }
        }

        if (executeSceneForeachCall(function, ownerObject, ownerClass, call, depth)) {
            return;
        }

        if (executeOwnerClassCall(ownerObject, ownerClass, call, depth)) {
            return;
        }

        if (executeClassConstructorCall(function, ownerObject, call, depth)) {
            return;
        }

        if (dispatchNative(function, ownerObject, ownerClass, call)) {
            return;
        }

        if (isSquirrelValueMethod(call.name)) {
            if (report_) {
                ++report_->engineObjectCalls;
            }
            recordEvent({"value_method_call", function.name, function.ordinal, ownerObject, ownerClass, call.name,
                receiver, "squirrel_value_method", {}, call.sourceLine, call.pc,
                "method on script/native return value; value VM needed for result"});
            return;
        }

        markUnresolved(call.name, receiver, ownerClass, call.sourceLine, call.pc, "no script/native binding");
    }

    bool executeReceiverCall(
        const std::string& receiver,
        const SqasmFunction& function,
        const std::string& ownerObject,
        const std::string& ownerClass,
        const SqasmCall& call,
        int depth)
    {
        const ScriptObjectMethodSlot* objectSlot = findObjectMethodSlot(objectMethodSlots_, receiver, call.name);
        if (objectSlot) {
            executeObjectMethodSlot(*objectSlot, receiver, call, depth);
            return true;
        }

        const auto objectClass = objectClasses_.find(receiver);
        if (objectClass != objectClasses_.end()) {
            const ScriptMethodBinding* method = findMethodBinding(methodBindings_, objectClass->second, call.name);
            if (method) {
                executeMethod(*method, receiver, objectClass->second, depth, "script_object_method");
                return true;
            }
        }

        if (hasClass(methodBindings_, receiver)) {
            const ScriptMethodBinding* method = findMethodBinding(methodBindings_, receiver, call.name);
            if (method) {
                const std::string targetObject = ownerObject.empty() ? receiver : ownerObject;
                executeMethod(*method, targetObject, receiver, depth, "script_super_or_class_method");
                return true;
            }
        }

        if (rootObjects_.find(receiver) != rootObjects_.end()) {
            if (report_) {
                ++report_->engineObjectCalls;
            }
            recordEvent({"engine_object_call", function.name, function.ordinal, ownerObject, ownerClass, call.name,
                receiver, "engine_object_method_unbound", {}, call.sourceLine, call.pc,
                "root object method has no recovered script slot"});
            return true;
        }

        if (receiver == "modMenu" || receiver == "modShop") {
            if (report_) {
                ++report_->optionalUnboundGlobals;
            }
            recordEvent({"optional_global_call", function.name, function.ordinal, ownerObject, ownerClass, call.name,
                receiver, "optional_unbound_global", {}, call.sourceLine, call.pc,
                "guarded by script truthiness check before call"});
            return true;
        }

        return false;
    }

    bool executeOwnerClassCall(
        const std::string& ownerObject,
        const std::string& ownerClass,
        const SqasmCall& call,
        int depth)
    {
        if (ownerClass.empty()) {
            return false;
        }
        const ScriptMethodBinding* method = findMethodBinding(methodBindings_, ownerClass, call.name);
        if (!method) {
            return false;
        }
        executeMethod(*method, ownerObject, ownerClass, depth, "script_owner_method");
        return true;
    }

    bool executeSceneForeachCall(
        const SqasmFunction& function,
        const std::string& ownerObject,
        const std::string& ownerClass,
        const SqasmCall& call,
        int depth)
    {
        if (ownerObject.empty() || ownerClass != "ModuleTitle" || function.name != "init" || call.name != "init") {
            return false;
        }
        const auto scenes = sceneObjectsByOwner_.find(ownerObject);
        if (scenes == sceneObjectsByOwner_.end()) {
            markUnresolved(call.name, {}, ownerClass, call.sourceLine, call.pc, "ModuleTitle._scenes not constructed");
            return true;
        }
        for (const auto& sceneObject : scenes->second) {
            const auto classIt = objectClasses_.find(sceneObject);
            if (classIt == objectClasses_.end()) {
                continue;
            }
            const ScriptMethodBinding* method = findMethodBinding(methodBindings_, classIt->second, "init");
            if (method) {
                executeMethod(*method, sceneObject, classIt->second, depth, "script_scene_foreach_method");
            }
        }
        return true;
    }

    bool executeClassConstructorCall(
        const SqasmFunction& function,
        const std::string& ownerObject,
        const SqasmCall& call,
        int depth)
    {
        if (!hasClass(methodBindings_, call.name)) {
            return false;
        }

        std::string objectName;
        if (function.name == "constructor" && !ownerObject.empty()) {
            objectName = sceneObjectNameForConstructorCall(function, call, ownerObject);
        } else {
            objectName = call.name + "@" + std::to_string(call.pc);
        }
        constructObject(
            objectName,
            call.name,
            call.pc,
            call.sourceLine,
            "class constructor call recovered from _OP_PREPCALLK",
            depth);
        return true;
    }

    bool dispatchNative(
        const SqasmFunction& function,
        const std::string& ownerObject,
        const std::string& ownerClass,
        const SqasmCall& call)
    {
        const native::ApiSurface* api = registry_.find(call.name);
        if (!api) {
            return false;
        }

        native::NativeCallContext context;
        context.module = module_.path.string();
        context.function = function.name;
        context.sourceLine = call.sourceLine;
        context.pc = call.pc;
        const native::NativeDispatchResult dispatched = catalog_.dispatch(*api, context);
        if (report_) {
            if (dispatched.implemented) {
                ++report_->nativeImplementedCalls;
            } else {
                ++report_->nativeObligations;
            }
            report_->obligations.push_back({dispatched.api, dispatched.service, dispatched.ownerLevel,
                dispatched.implementationStatus, function.name, function.ordinal, call.sourceLine, call.pc,
                dispatched.obligation});
        }
        recordEvent({"native_dispatch", function.name, function.ordinal, ownerObject, ownerClass, call.name, {},
            dispatched.implemented ? "native_implemented" : "native_obligation", dispatched.service, call.sourceLine,
            call.pc, dispatched.obligation});
        return true;
    }

    void markUnresolved(
        const std::string& callName,
        const std::string& receiver,
        const std::string& ownerClass,
        int sourceLine,
        int pc,
        const std::string& evidence)
    {
        if (report_) {
            ++report_->unresolvedCalls;
        }
        recordEvent({"unresolved_call", {}, -1, {}, ownerClass, callName, receiver, "unresolved", {}, sourceLine, pc,
            evidence});
    }

    const SqasmModule& module_;
    const native::NativeRegistry& registry_;
    const native::NativeServiceCatalog& catalog_;
    std::vector<ScriptMethodBinding> methodBindings_;
    std::vector<ScriptObjectBinding> objectBindings_;
    std::vector<ScriptObjectMethodSlot> objectMethodSlots_;
    std::vector<FunctionSlotBinding> functionSlots_;
    std::map<std::string, std::string> objectClasses_;
    std::map<std::string, std::vector<std::string>> sceneObjectsByOwner_;
    std::set<std::string> rootObjects_;
    std::set<std::string> activeMethods_;
    ScriptExecutionReport* report_ = nullptr;
    static constexpr int maxDepth_ = 24;
    static constexpr size_t maxEvents_ = 5000;
};

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

ScriptExecutionReport runEntryScript(
    const SqasmModule& module,
    const std::string& entryFunction,
    const native::NativeRegistry& registry,
    const native::NativeServiceCatalog& catalog,
    int frames)
{
    StaticScriptExecutor executor(module, registry, catalog);
    return executor.run(entryFunction, frames);
}

std::string scriptExecutionReportToJson(const ScriptExecutionReport& report)
{
    std::ostringstream out;
    std::set<std::string> uniqueNativeApis;
    for (const auto& obligation : report.obligations) {
        uniqueNativeApis.insert(obligation.api);
    }

    out << "{\n";
    out << "  \"schema\": \"yuengine.script_execution_report.v1\",\n";
    out << "  \"module\": \"" << core::jsonEscape(report.modulePath) << "\",\n";
    out << "  \"entry_function\": \"" << core::jsonEscape(report.entryFunction) << "\",\n";
    out << "  \"metrics\": \"entry=" << core::jsonEscape(report.entryFunction)
        << " found=" << (report.entryFound ? "true" : "false")
        << " executed=" << (report.executed ? "true" : "false") << " frames=" << report.frames
        << " constructed_objects=" << report.constructedObjects << " script_methods=" << report.scriptMethods
        << " script_functions=" << report.scriptFunctions << " builtin_calls=" << report.builtinCalls
        << " native_obligations=" << report.nativeObligations
        << " native_implemented_calls=" << report.nativeImplementedCalls
        << " unique_native_apis=" << uniqueNativeApis.size() << " engine_object_calls="
        << report.engineObjectCalls << " optional_unbound_globals=" << report.optionalUnboundGlobals
        << " unresolved_calls=" << report.unresolvedCalls << " truncated="
        << (report.truncated ? "true" : "false") << " status=" << core::jsonEscape(report.status) << "\",\n";
    out << "  \"entry_found\": " << (report.entryFound ? "true" : "false") << ",\n";
    out << "  \"executed\": " << (report.executed ? "true" : "false") << ",\n";
    out << "  \"frames\": " << report.frames << ",\n";
    out << "  \"status\": \"" << core::jsonEscape(report.status) << "\",\n";
    out << "  \"execution_mode\": \"" << core::jsonEscape(report.executionMode) << "\",\n";
    out << "  \"class_method_tables\": " << report.classMethodTableCount << ",\n";
    out << "  \"method_bindings\": " << report.methodBindingCount << ",\n";
    out << "  \"object_bindings\": " << report.objectBindingCount << ",\n";
    out << "  \"root_object_method_slots\": " << report.rootObjectMethodSlots << ",\n";
    out << "  \"constructed_objects\": " << report.constructedObjects << ",\n";
    out << "  \"script_methods\": " << report.scriptMethods << ",\n";
    out << "  \"script_functions\": " << report.scriptFunctions << ",\n";
    out << "  \"builtin_calls\": " << report.builtinCalls << ",\n";
    out << "  \"native_obligations\": " << report.nativeObligations << ",\n";
    out << "  \"native_implemented_calls\": " << report.nativeImplementedCalls << ",\n";
    out << "  \"unique_native_apis\": " << uniqueNativeApis.size() << ",\n";
    out << "  \"engine_object_calls\": " << report.engineObjectCalls << ",\n";
    out << "  \"optional_unbound_globals\": " << report.optionalUnboundGlobals << ",\n";
    out << "  \"unresolved_calls\": " << report.unresolvedCalls << ",\n";
    out << "  \"truncated\": " << (report.truncated ? "true" : "false") << ",\n";
    out << "  \"unique_native_api_names\": ";
    const std::vector<std::string> uniqueNativeApiNames(uniqueNativeApis.begin(), uniqueNativeApis.end());
    writeStringArray(out, uniqueNativeApiNames);
    out << ",\n";
    out << "  \"constructed_object_details\": [\n";
    for (size_t i = 0; i < report.constructedObjectDetails.size(); ++i) {
        const auto& object = report.constructedObjectDetails[i];
        out << "    {\"object\": \"" << core::jsonEscape(object.objectName) << "\", \"class\": \""
            << core::jsonEscape(object.className) << "\", \"pc\": " << object.pc << ", \"source_line\": "
            << object.sourceLine << ", \"evidence\": \"" << core::jsonEscape(object.evidence) << "\"}";
        out << (i + 1 == report.constructedObjectDetails.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"root_object_method_slot_details\": [\n";
    for (size_t i = 0; i < report.objectMethodSlots.size(); ++i) {
        const auto& slot = report.objectMethodSlots[i];
        out << "    {\"object\": \"" << core::jsonEscape(slot.objectName) << "\", \"slot\": \""
            << core::jsonEscape(slot.slot) << "\", \"function_name\": \"" << core::jsonEscape(slot.functionName)
            << "\", \"function_ref_index\": " << slot.functionRefIndex << ", \"function_ordinal\": "
            << slot.functionOrdinal << ", \"source_line\": " << slot.sourceLine << ", \"evidence\": \""
            << core::jsonEscape(slot.evidence) << "\"}";
        out << (i + 1 == report.objectMethodSlots.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"native_obligation_details\": [\n";
    for (size_t i = 0; i < report.obligations.size(); ++i) {
        const auto& obligation = report.obligations[i];
        out << "    {\"api\": \"" << core::jsonEscape(obligation.api) << "\", \"service\": \""
            << core::jsonEscape(obligation.service) << "\", \"owner_level\": \""
            << core::jsonEscape(obligation.ownerLevel) << "\", \"implementation_status\": \""
            << core::jsonEscape(obligation.implementationStatus) << "\", \"function\": \""
            << core::jsonEscape(obligation.functionName) << "\", \"function_ordinal\": "
            << obligation.functionOrdinal << ", \"source_line\": " << obligation.sourceLine << ", \"pc\": "
            << obligation.pc << ", \"obligation\": \"" << core::jsonEscape(obligation.obligation) << "\"}";
        out << (i + 1 == report.obligations.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"events\": [\n";
    for (size_t i = 0; i < report.events.size(); ++i) {
        const auto& event = report.events[i];
        out << "    {\"kind\": \"" << core::jsonEscape(event.kind) << "\", \"function\": \""
            << core::jsonEscape(event.functionName) << "\", \"function_ordinal\": " << event.functionOrdinal
            << ", \"owner_object\": \"" << core::jsonEscape(event.ownerObject) << "\", \"owner_class\": \""
            << core::jsonEscape(event.ownerClass) << "\", \"call\": \"" << core::jsonEscape(event.callName)
            << "\", \"receiver\": \"" << core::jsonEscape(event.receiver) << "\", \"category\": \""
            << core::jsonEscape(event.category) << "\", \"service\": \"" << core::jsonEscape(event.service)
            << "\", \"source_line\": " << event.sourceLine << ", \"pc\": " << event.pc << ", \"evidence\": \""
            << core::jsonEscape(event.evidence) << "\"}";
        out << (i + 1 == report.events.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

} // namespace yu::script
