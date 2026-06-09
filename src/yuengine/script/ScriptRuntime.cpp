#include "yuengine/script/ScriptRuntime.h"

#include "yuengine/core/Json.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

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

enum class ScriptValueKind {
    Unknown,
    Null,
    Bool,
    Int,
    Float,
    Expression,
    String,
    Root,
    Object,
    Class,
    Table,
    Function,
    NativeFunction,
    Method,
    ValueMethod,
    UiMethod,
    Parameter,
    Vector2,
    Vector3,
    Quaternion,
    SaveList,
    SaveEntry,
    ScenarioKeys,
    Mission,
    MissionRequest,
    Marker,
    Actor,
    Loader,
    PlaceParams,
    CameraTask,
};

enum class Truthiness {
    Unknown,
    False,
    True,
};

struct ScriptValue {
    ScriptValueKind kind = ScriptValueKind::Unknown;
    std::string text;
    std::string receiver;
    int intValue = 0;
    double numberValue = 0.0;
    bool boolValue = false;
    int tableId = -1;
    int functionOrdinal = -1;
};

struct BytecodeStateResult {
    std::set<int> executedCallPcs;
    std::set<int> synchronouslyExecutedCallPcs;
    std::map<int, std::string> callReceivers;
    std::map<int, std::vector<ScriptValue>> callArguments;
    ScriptValue returnValue;
    int executedInstructions = 0;
    bool controlFlowUnknown = false;
};

struct SynchronousCallResult {
    bool handled = false;
    ScriptValue value;
};

struct RuntimeObject {
    std::string className;
    std::string runtimeType;
    std::map<std::string, ScriptValue> fields;
};

struct RuntimeTable {
    std::map<std::string, ScriptValue> fields;
};

ScriptValue unknownValue()
{
    return {};
}

ScriptValue nullValue()
{
    ScriptValue value;
    value.kind = ScriptValueKind::Null;
    return value;
}

ScriptValue boolValue(bool data)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Bool;
    value.boolValue = data;
    return value;
}

ScriptValue intValue(int data)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Int;
    value.intValue = data;
    value.numberValue = static_cast<double>(data);
    return value;
}

ScriptValue floatValue(double data)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Float;
    value.numberValue = data;
    return value;
}

ScriptValue expressionValue(std::string data)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Expression;
    value.text = std::move(data);
    return value;
}

ScriptValue stringValue(std::string data)
{
    ScriptValue value;
    value.kind = ScriptValueKind::String;
    value.text = std::move(data);
    return value;
}

ScriptValue rootValue()
{
    ScriptValue value;
    value.kind = ScriptValueKind::Root;
    value.text = "<root>";
    return value;
}

ScriptValue objectValue(std::string objectName)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Object;
    value.text = std::move(objectName);
    return value;
}

ScriptValue classValue(std::string className)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Class;
    value.text = std::move(className);
    return value;
}

ScriptValue tableValue(int tableId)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Table;
    value.tableId = tableId;
    value.text = "table#" + std::to_string(tableId);
    return value;
}

ScriptValue functionValue(std::string functionName, int ordinal)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Function;
    value.text = std::move(functionName);
    value.functionOrdinal = ordinal;
    return value;
}

ScriptValue nativeFunctionValue(std::string functionName, std::string receiver = {})
{
    ScriptValue value;
    value.kind = ScriptValueKind::NativeFunction;
    value.text = std::move(functionName);
    value.receiver = std::move(receiver);
    return value;
}

ScriptValue methodValue(std::string methodName, std::string receiver)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Method;
    value.text = std::move(methodName);
    value.receiver = std::move(receiver);
    return value;
}

ScriptValue valueMethodValue(std::string methodName, std::string receiver)
{
    ScriptValue value;
    value.kind = ScriptValueKind::ValueMethod;
    value.text = std::move(methodName);
    value.receiver = std::move(receiver);
    return value;
}

ScriptValue uiMethodValue(std::string methodName, std::string receiver)
{
    ScriptValue value;
    value.kind = ScriptValueKind::UiMethod;
    value.text = std::move(methodName);
    value.receiver = std::move(receiver);
    return value;
}

ScriptValue parameterValue(std::string name)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Parameter;
    value.text = std::move(name);
    return value;
}

ScriptValue vector2Value(std::string label)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Vector2;
    value.text = std::move(label);
    return value;
}

ScriptValue vector3Value(std::string label)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Vector3;
    value.text = std::move(label);
    return value;
}

ScriptValue quaternionValue(std::string label)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Quaternion;
    value.text = std::move(label);
    return value;
}

ScriptValue saveListValue()
{
    ScriptValue value;
    value.kind = ScriptValueKind::SaveList;
    value.text = "save_list";
    return value;
}

ScriptValue saveEntryValue(std::string label)
{
    ScriptValue value;
    value.kind = ScriptValueKind::SaveEntry;
    value.text = std::move(label);
    return value;
}

ScriptValue missionRequestValue(std::string label)
{
    ScriptValue value;
    value.kind = ScriptValueKind::MissionRequest;
    value.text = std::move(label);
    return value;
}

ScriptValue markerValue(std::string label)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Marker;
    value.text = std::move(label);
    return value;
}

ScriptValue actorValue(std::string label)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Actor;
    value.text = std::move(label);
    return value;
}

ScriptValue loaderValue(std::string label)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Loader;
    value.text = std::move(label);
    return value;
}

ScriptValue placeParamsValue(std::string label)
{
    ScriptValue value;
    value.kind = ScriptValueKind::PlaceParams;
    value.text = std::move(label);
    return value;
}

ScriptValue cameraTaskValue(std::string label)
{
    ScriptValue value;
    value.kind = ScriptValueKind::CameraTask;
    value.text = std::move(label);
    return value;
}

ScriptValue scenarioKeysValue()
{
    ScriptValue value;
    value.kind = ScriptValueKind::ScenarioKeys;
    value.text = "scenario_keys";
    return value;
}

ScriptValue missionValue(std::string label)
{
    ScriptValue value;
    value.kind = ScriptValueKind::Mission;
    value.text = std::move(label);
    return value;
}

bool isKnownValue(const ScriptValue& value)
{
    return value.kind != ScriptValueKind::Unknown;
}

bool isConcreteReceiver(const std::string& receiver)
{
    return !receiver.empty() && receiver != "unknown" && receiver.rfind("unknown ", 0) != 0;
}

std::string slotKey(const ScriptValue& value)
{
    switch (value.kind) {
    case ScriptValueKind::String:
    case ScriptValueKind::Class:
    case ScriptValueKind::Object:
    case ScriptValueKind::Parameter:
        return value.text;
    case ScriptValueKind::Int:
        return std::to_string(value.intValue);
    default:
        return {};
    }
}

std::string valueKindName(ScriptValueKind kind)
{
    switch (kind) {
    case ScriptValueKind::Unknown:
        return "unknown";
    case ScriptValueKind::Null:
        return "null";
    case ScriptValueKind::Bool:
        return "bool";
    case ScriptValueKind::Int:
        return "int";
    case ScriptValueKind::Float:
        return "float";
    case ScriptValueKind::Expression:
        return "expression";
    case ScriptValueKind::String:
        return "string";
    case ScriptValueKind::Root:
        return "root";
    case ScriptValueKind::Object:
        return "object";
    case ScriptValueKind::Class:
        return "class";
    case ScriptValueKind::Table:
        return "table";
    case ScriptValueKind::Function:
        return "function";
    case ScriptValueKind::NativeFunction:
        return "native_function";
    case ScriptValueKind::Method:
        return "method";
    case ScriptValueKind::ValueMethod:
        return "value_method";
    case ScriptValueKind::UiMethod:
        return "ui_method";
    case ScriptValueKind::Parameter:
        return "parameter";
    case ScriptValueKind::Vector2:
        return "vector2";
    case ScriptValueKind::Vector3:
        return "vector3";
    case ScriptValueKind::Quaternion:
        return "quaternion";
    case ScriptValueKind::SaveList:
        return "save_list";
    case ScriptValueKind::SaveEntry:
        return "save_entry";
    case ScriptValueKind::ScenarioKeys:
        return "scenario_keys";
    case ScriptValueKind::Mission:
        return "mission";
    case ScriptValueKind::MissionRequest:
        return "mission_request";
    case ScriptValueKind::Marker:
        return "marker";
    case ScriptValueKind::Actor:
        return "actor";
    case ScriptValueKind::Loader:
        return "loader";
    case ScriptValueKind::PlaceParams:
        return "place_params";
    case ScriptValueKind::CameraTask:
        return "camera_task";
    }
    return "unknown";
}

std::string conciseNumber(double value)
{
    std::ostringstream out;
    out << value;
    return out.str();
}

std::string valueSummary(const ScriptValue& value)
{
    switch (value.kind) {
    case ScriptValueKind::Unknown:
        return "unknown";
    case ScriptValueKind::Null:
        return "null";
    case ScriptValueKind::Bool:
        return value.boolValue ? "true" : "false";
    case ScriptValueKind::Int:
        return std::to_string(value.intValue);
    case ScriptValueKind::Float:
        return conciseNumber(value.numberValue);
    case ScriptValueKind::Expression:
    case ScriptValueKind::String:
    case ScriptValueKind::Root:
    case ScriptValueKind::Object:
    case ScriptValueKind::Class:
    case ScriptValueKind::Table:
    case ScriptValueKind::Function:
    case ScriptValueKind::NativeFunction:
    case ScriptValueKind::Method:
    case ScriptValueKind::ValueMethod:
    case ScriptValueKind::UiMethod:
    case ScriptValueKind::Parameter:
    case ScriptValueKind::Vector2:
    case ScriptValueKind::Vector3:
    case ScriptValueKind::Quaternion:
    case ScriptValueKind::SaveList:
    case ScriptValueKind::SaveEntry:
    case ScriptValueKind::ScenarioKeys:
    case ScriptValueKind::Mission:
    case ScriptValueKind::MissionRequest:
    case ScriptValueKind::Marker:
    case ScriptValueKind::Actor:
    case ScriptValueKind::Loader:
    case ScriptValueKind::PlaceParams:
    case ScriptValueKind::CameraTask:
        return value.text.empty() ? valueKindName(value.kind) : value.text;
    }
    return "unknown";
}

std::string argumentValueText(const ScriptValue& value)
{
    return value.kind == ScriptValueKind::String ? value.text : valueSummary(value);
}

std::string argumentSummary(const std::vector<ScriptValue>& arguments)
{
    if (arguments.empty()) {
        return {};
    }
    std::ostringstream out;
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i != 0) {
            out << "; ";
        }
        out << "arg" << i << "=" << argumentValueText(arguments[i]);
    }
    return out.str();
}

std::string positionalArgumentSummary(const std::vector<ScriptValue>& arguments)
{
    std::ostringstream out;
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i != 0) {
            out << ", ";
        }
        out << argumentValueText(arguments[i]);
    }
    return out.str();
}

bool hasDecodedArgumentPayload(const std::string& arguments)
{
    return !arguments.empty() && arguments.find("pending") == std::string::npos;
}

std::string describeValue(const ScriptValue& value)
{
    std::ostringstream out;
    out << valueKindName(value.kind);
    if (!value.text.empty()) {
        out << ":" << value.text;
    } else if (value.kind == ScriptValueKind::Bool) {
        out << ":" << (value.boolValue ? "true" : "false");
    } else if (value.kind == ScriptValueKind::Int) {
        out << ":" << value.intValue;
    } else if (value.kind == ScriptValueKind::Float) {
        out << ":" << value.numberValue;
    }
    if (!value.receiver.empty()) {
        out << " receiver=" << value.receiver;
    }
    return out.str();
}

void writeScriptValueJson(std::ostringstream& out, const ScriptValue& value)
{
    out << "{\"kind\": \"" << valueKindName(value.kind) << "\"";
    if (!value.text.empty()) {
        out << ", \"text\": \"" << core::jsonEscape(value.text) << "\"";
    }
    if (!value.receiver.empty()) {
        out << ", \"receiver\": \"" << core::jsonEscape(value.receiver) << "\"";
    }
    if (value.kind == ScriptValueKind::Bool) {
        out << ", \"bool\": " << (value.boolValue ? "true" : "false");
    } else if (value.kind == ScriptValueKind::Int) {
        out << ", \"int\": " << value.intValue;
    } else if (value.kind == ScriptValueKind::Float) {
        out << ", \"number\": " << value.numberValue;
    } else if (value.kind == ScriptValueKind::Table) {
        out << ", \"table_id\": " << value.tableId;
    } else if (value.kind == ScriptValueKind::Function) {
        out << ", \"function_ordinal\": " << value.functionOrdinal;
    }
    out << "}";
}

void writeScriptValueMapJson(std::ostringstream& out, const std::map<std::string, ScriptValue>& values)
{
    out << "{";
    size_t index = 0;
    for (const auto& [key, value] : values) {
        out << (index++ == 0 ? "" : ", ") << "\"" << core::jsonEscape(key) << "\": ";
        writeScriptValueJson(out, value);
    }
    out << "}";
}

void writeStringStringMapJson(std::ostringstream& out, const std::map<std::string, std::string>& values)
{
    out << "{";
    size_t index = 0;
    for (const auto& [key, value] : values) {
        out << (index++ == 0 ? "" : ", ") << "\"" << core::jsonEscape(key) << "\": \""
            << core::jsonEscape(value) << "\"";
    }
    out << "}";
}

void writeRuntimeObjectsJson(std::ostringstream& out, const std::map<std::string, RuntimeObject>& objects)
{
    out << "[";
    size_t index = 0;
    for (const auto& [name, object] : objects) {
        out << (index++ == 0 ? "" : ", ") << "{\"name\": \"" << core::jsonEscape(name)
            << "\", \"class\": \"" << core::jsonEscape(object.className)
            << "\", \"runtime_type\": \"" << core::jsonEscape(object.runtimeType)
            << "\", \"field_count\": " << object.fields.size() << ", \"fields\": ";
        writeScriptValueMapJson(out, object.fields);
        out << "}";
    }
    out << "]";
}

void writeRuntimeTablesJson(std::ostringstream& out, const std::map<int, RuntimeTable>& tables)
{
    out << "[";
    size_t index = 0;
    for (const auto& [id, table] : tables) {
        out << (index++ == 0 ? "" : ", ") << "{\"id\": " << id << ", \"field_count\": "
            << table.fields.size() << ", \"fields\": ";
        writeScriptValueMapJson(out, table.fields);
        out << "}";
    }
    out << "]";
}

void writeClassSlotsJson(
    std::ostringstream& out,
    const std::map<std::string, std::map<std::string, ScriptValue>>& classSlots)
{
    out << "{";
    size_t index = 0;
    for (const auto& [name, slots] : classSlots) {
        out << (index++ == 0 ? "" : ", ") << "\"" << core::jsonEscape(name) << "\": {\"slot_count\": "
            << slots.size() << ", \"slots\": ";
        writeScriptValueMapJson(out, slots);
        out << "}";
    }
    out << "}";
}

bool numericValue(const ScriptValue& value, double& out)
{
    if (value.kind == ScriptValueKind::Int) {
        out = static_cast<double>(value.intValue);
        return true;
    }
    if (value.kind == ScriptValueKind::Float) {
        out = value.numberValue;
        return true;
    }
    return false;
}

bool parseNumberText(const std::string& text, double& out)
{
    if (text.empty()) {
        return false;
    }
    try {
        size_t parsed = 0;
        out = std::stod(text, &parsed);
        return parsed == text.size();
    } catch (...) {
        return false;
    }
}

int signedBytecodeDelta(int encoded)
{
    return encoded > 127 ? encoded - 256 : encoded;
}

const std::vector<std::string>& scenarioKeys()
{
    static const std::vector<std::string> keys = {"sc01", "sc02", "sc03"};
    return keys;
}

std::string scenarioKeyAtIndex(int index)
{
    const auto& keys = scenarioKeys();
    if (keys.empty()) {
        return {};
    }
    if (index < 0) {
        index = 0;
    }
    if (static_cast<size_t>(index) >= keys.size()) {
        index = static_cast<int>(keys.size() - 1);
    }
    return keys[static_cast<size_t>(index)];
}

std::string scenarioKeyFromArguments(const std::vector<ScriptValue>& arguments)
{
    if (arguments.empty()) {
        return scenarioKeyAtIndex(0);
    }

    double index = 0.0;
    if (numericValue(arguments[0], index)) {
        return scenarioKeyAtIndex(static_cast<int>(index));
    }

    const std::string text = valueSummary(arguments[0]);
    return text.empty() || text == "unknown" ? scenarioKeyAtIndex(0) : text;
}

std::string missionLabelForScenarioKey(const std::string& scenarioKey)
{
    return "mission:" + (scenarioKey.empty() ? scenarioKeyAtIndex(0) : scenarioKey) + "/main/ms010_0";
}

constexpr const char* kFirstMissionScript = "mission/sc01/main/ms010_0.b64.sqasm";
constexpr const char* kFirstMissionStage = "map/Doujou/doujou.sge";
constexpr const char* kFirstMissionRailCamera = "map/Doujou/doujou.rcm";
constexpr const char* kFirstMissionEventScript = "sc01/main/ms010_0";
constexpr const char* kFirstMissionEventObject = "ev_sc01_main_ms010_0";
constexpr const char* kFirstMissionPlayer = "reimuEx";
constexpr const char* kFirstMissionMarker = "marker:sc01/main/ms010_0:eventMap";
constexpr const char* kFirstMissionPlayerPos = "marker:sc01/main/ms010_0:eventMap._pos";
constexpr const char* kFirstMissionPlayerRot = "marker:sc01/main/ms010_0:eventMap._rot";

ScriptValue tableKeyValue(const std::string& key)
{
    try {
        size_t parsed = 0;
        const int value = std::stoi(key, &parsed);
        if (parsed == key.size()) {
            return intValue(value);
        }
    } catch (...) {
    }
    return stringValue(key);
}

Truthiness equalityTruthiness(const ScriptValue& lhs, const ScriptValue& rhs)
{
    double lhsNumber = 0.0;
    double rhsNumber = 0.0;
    if (numericValue(lhs, lhsNumber) && numericValue(rhs, rhsNumber)) {
        return lhsNumber == rhsNumber ? Truthiness::True : Truthiness::False;
    }
    if (lhs.kind == ScriptValueKind::Bool && rhs.kind == ScriptValueKind::Bool) {
        return lhs.boolValue == rhs.boolValue ? Truthiness::True : Truthiness::False;
    }
    if ((lhs.kind == ScriptValueKind::String || lhs.kind == ScriptValueKind::Expression)
        && (rhs.kind == ScriptValueKind::String || rhs.kind == ScriptValueKind::Expression)) {
        return lhs.text == rhs.text ? Truthiness::True : Truthiness::False;
    }
    if (lhs.kind == ScriptValueKind::Null || rhs.kind == ScriptValueKind::Null) {
        return lhs.kind == rhs.kind ? Truthiness::True : Truthiness::False;
    }
    return Truthiness::Unknown;
}

bool valuesEqual(const ScriptValue& lhs, const ScriptValue& rhs)
{
    return equalityTruthiness(lhs, rhs) == Truthiness::True;
}

Truthiness valueTruthiness(const ScriptValue& value)
{
    switch (value.kind) {
    case ScriptValueKind::Unknown:
    case ScriptValueKind::Expression:
    case ScriptValueKind::Parameter:
        return Truthiness::Unknown;
    case ScriptValueKind::Null:
        return Truthiness::False;
    case ScriptValueKind::Bool:
        return value.boolValue ? Truthiness::True : Truthiness::False;
    case ScriptValueKind::Int:
        return value.intValue != 0 ? Truthiness::True : Truthiness::False;
    case ScriptValueKind::Float:
        return value.numberValue != 0.0 ? Truthiness::True : Truthiness::False;
    case ScriptValueKind::String:
        return value.text.empty() ? Truthiness::False : Truthiness::True;
    default:
        return Truthiness::True;
    }
}

bool truthyValue(const ScriptValue& value)
{
    return valueTruthiness(value) == Truthiness::True;
}

ScriptValue arithmeticValue(const ScriptValue& lhs, const ScriptValue& rhs, int op)
{
    double lhsNumber = 0.0;
    double rhsNumber = 0.0;
    const bool lhsIsNumber = numericValue(lhs, lhsNumber);
    const bool rhsIsNumber = numericValue(rhs, rhsNumber);
    if (lhsIsNumber && rhsIsNumber) {
        if (op == 43) {
            return floatValue(lhsNumber + rhsNumber);
        }
        if (op == 45) {
            return floatValue(lhsNumber - rhsNumber);
        }
        if (op == 42) {
            return floatValue(lhsNumber * rhsNumber);
        }
        if (op == 44 && rhsNumber != 0.0) {
            return floatValue(lhsNumber / rhsNumber);
        }
    }

    std::string symbol = "?";
    if (op == 43) {
        symbol = "+";
    } else if (op == 45) {
        symbol = "-";
    } else if (op == 42) {
        symbol = "*";
    } else if (op == 44) {
        symbol = "/";
    }
    return expressionValue("(" + valueSummary(lhs) + " " + symbol + " " + valueSummary(rhs) + ")");
}

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
            binding.modulePath = module.path.string();
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

std::vector<ScriptMethodBinding> buildCombinedMethodBindings(
    const SqasmModule& module,
    const std::vector<SqasmModule>& baselineModules)
{
    std::vector<ScriptMethodBinding> bindings = buildMethodBindings(module);
    for (const auto& baselineModule : baselineModules) {
        std::vector<ScriptMethodBinding> baselineBindings = buildMethodBindings(baselineModule);
        bindings.insert(bindings.end(), baselineBindings.begin(), baselineBindings.end());
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
    static const std::set<std::string> methods = {
        "checkFall",
        "count",
        "get",
        "getRequest",
        "isActive",
        "toQuat",
        "toVec3",
        "toAbsPos",
        "tofloat",
        "tointeger",
        "toPoint",
    };
    return methods.find(name) != methods.end();
}

bool isScriptFieldReceiver(const std::string& receiver)
{
    return !receiver.empty() && receiver.front() == '_';
}

bool isUiObjectMethod(const std::string& name)
{
    static const std::set<std::string> methods = {
        "bl",
        "init",
        "renderHorizontal",
        "setParent",
        "setSelectCursor",
        "tr",
    };
    return methods.find(name) != methods.end();
}

bool isRuntimeOwnedScriptMethod(const std::string& name)
{
    static const std::set<std::string> methods = {
        "getDrawEnd",
        "init",
        "move",
        "resetAnim",
        "selectCursorX",
        "selectCursorY",
        "setFadeIn",
        "setParent",
        "setSelectCursor",
    };
    return methods.find(name) != methods.end();
}

bool isValueHelperCall(const std::string& name)
{
    static const std::set<std::string> helpers = {
        "centerPos",
        "float2",
        "Vec3",
    };
    return helpers.find(name) != helpers.end();
}

bool isActorRuntimeMethod(const std::string& name)
{
    static const std::set<std::string> methods = {
        "fillHealProgress",
        "playEffect",
        "setArmed",
        "setWaitForLanding",
    };
    return methods.find(name) != methods.end();
}

bool isLoaderRuntimeMethod(const std::string& name)
{
    return name == "end";
}

bool isPlaceParamsRuntimeMethod(const std::string& name)
{
    static const std::set<std::string> methods = {
        "setEnabledRetWorld",
        "setLabel",
        "setSlaveDisp",
    };
    return methods.find(name) != methods.end();
}

bool isMissionRuntimeMethod(const std::string& name)
{
    return name == "checkFall";
}

bool isModuleLifecycleCall(const std::string& ownerClass, const std::string& name)
{
    return ownerClass.rfind("Module", 0) == 0 && name == "stateInit";
}

bool isReturnSynchronizedScriptClass(const std::string& className)
{
    if (className.rfind("Module", 0) == 0 || className == "TitleSceneBase") {
        return true;
    }
    const std::string suffix = "Scene";
    return className.size() >= suffix.size()
        && className.compare(className.size() - suffix.size(), suffix.size(), suffix) == 0;
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
        const std::vector<SqasmModule>& baselineModules,
        const native::NativeRegistry& registry,
        const native::NativeServiceCatalog& catalog,
        ScriptRunOptions options)
        : module_(module)
        , baselineModules_(baselineModules)
        , registry_(registry)
        , catalog_(catalog)
        , options_(std::move(options))
        , methodBindings_(buildCombinedMethodBindings(module, baselineModules))
        , objectBindings_(buildObjectBindings(module, methodBindings_))
        , objectMethodSlots_(buildRootObjectMethodSlots(module))
        , functionSlots_(buildRootFunctionSlots(module))
    {
        for (const auto& binding : objectMethodSlots_) {
            rootObjects_.insert(binding.objectName);
        }
    }

    ScriptExecutionReport run(const std::string& entryFunction)
    {
        catalog_.resetRuntimeState();
        ScriptExecutionReport report;
        report.modulePath = module_.path.string();
        report.entryFunction = entryFunction;
        report.frames = options_.frames < 0 ? 0 : options_.frames;
        report.executionMode = "multi_module_bytecode_state_plus_static_call_trace_branch_sensitive_boot_edge";
        report.baselineModules = static_cast<int>(baselineModules_.size());
        report.runtimeInputStateJson = runtimeInputStateToJson();
        for (const auto& baselineModule : baselineModules_) {
            report.baselineModulePaths.push_back(baselineModule.path.string());
        }
        report.classMethodTableCount = static_cast<int>(classNamesFromMethodBindings(methodBindings_).size());
        report.methodBindingCount = static_cast<int>(methodBindings_.size());
        report.objectBindingCount = static_cast<int>(objectBindings_.size());
        report.rootObjectMethodSlots = static_cast<int>(objectMethodSlots_.size());
        report.objectMethodSlots = objectMethodSlots_;
        report_ = &report;

        const SqasmFunction* entry = findEntrypoint(entryFunction);
        if (!entry) {
            report.status = "entry_not_found_or_ambiguous";
            report.runtimeServiceStateJson = catalog_.runtimeStateToJson();
            report.runtimeScriptStateJson = runtimeScriptStateToJson();
            report_ = nullptr;
            return report;
        }

        report.entryFound = true;
        bootstrapRootState();
        bootstrapRootObjects();
        executeFunction(*entry, {}, {}, 0);
        for (int frame = 0; frame < report.frames; ++frame) {
            executeGlobalFunctionSlot("main", 0);
        }

        report.constructedObjects = static_cast<int>(report.constructedObjectDetails.size());
        report.executed = true;
        report.status = report.truncated ? "trace_truncated_not_full_vm" : "trace_ready_not_full_vm";
        report.runtimeServiceStateJson = catalog_.runtimeStateToJson();
        report.runtimeScriptStateJson = runtimeScriptStateToJson();
        report_ = nullptr;
        return report;
    }

private:
    std::string runtimeInputStateToJson() const
    {
        std::ostringstream out;
        out << "{";
        out << "\"scenario\": \"" << core::jsonEscape(options_.inputScenario) << "\", ";
        out << "\"menu_selected_index\": " << options_.menuSelectedIndex << ", ";
        out << "\"menu_decide\": " << (options_.menuDecide ? "true" : "false") << ", ";
        out << "\"menu_down\": " << (options_.menuDown ? "true" : "false") << ", ";
        out << "\"menu_up\": " << (options_.menuUp ? "true" : "false") << ", ";
        out << "\"save_list_empty\": " << (options_.saveListEmpty ? "true" : "false") << ", ";
        out << "\"continue_disabled\": " << (options_.continueDisabled ? "true" : "false") << ", ";
        out << "\"can_shutdown\": " << (options_.canShutdown ? "true" : "false");
        out << "}";
        return out.str();
    }

    const SqasmModule& moduleForBinding(const ScriptMethodBinding& binding) const
    {
        if (binding.modulePath.empty() || binding.modulePath == module_.path.string()) {
            return module_;
        }
        for (const auto& baselineModule : baselineModules_) {
            if (binding.modulePath == baselineModule.path.string()) {
                return baselineModule;
            }
        }
        return module_;
    }

    std::string runtimeScriptStateToJson() const
    {
        std::ostringstream out;
        out << "{";
        out << "\"root_field_count\": " << rootFields_.size() << ", ";
        out << "\"object_count\": " << runtimeObjects_.size() << ", ";
        out << "\"table_count\": " << runtimeTables_.size() << ", ";
        out << "\"class_slot_table_count\": " << classSlots_.size() << ", ";
        out << "\"class_base_count\": " << classBases_.size() << ", ";
        out << "\"root_fields\": ";
        writeScriptValueMapJson(out, rootFields_);
        out << ", \"class_bases\": ";
        writeStringStringMapJson(out, classBases_);
        out << ", \"objects\": ";
        writeRuntimeObjectsJson(out, runtimeObjects_);
        out << ", \"tables\": ";
        writeRuntimeTablesJson(out, runtimeTables_);
        out << ", \"class_slots\": ";
        writeClassSlotsJson(out, classSlots_);
        out << "}";
        return out.str();
    }

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

    void bootstrapRootState()
    {
        ensureRuntimeObject("<root>", {}, "root_table");
        rootFields_["gMenu"] = objectValue("gMenu");
        ensureRuntimeObject("gMenu", "EngineMenuRoot", "engine_root_object");
        for (const auto& baselineModule : baselineModules_) {
            executeRootBytecodeState(baselineModule);
        }
        executeRootBytecodeState(module_);
    }

    void executeRootBytecodeState(const SqasmModule& module)
    {
        const SqasmFunction* root = findRootFunction(module);
        if (root) {
            executeBytecodeState(*root, {}, {}, 0);
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

    RuntimeObject& ensureRuntimeObject(
        const std::string& objectName,
        const std::string& className,
        const std::string& runtimeType)
    {
        auto& object = runtimeObjects_[objectName];
        if (!className.empty() && object.className.empty()) {
            object.className = className;
        }
        if (!runtimeType.empty()) {
            object.runtimeType = runtimeType;
        }
        return object;
    }

    void materializeClassDefaults(const std::string& objectName, const std::string& className)
    {
        if (objectName.empty() || className.empty()) {
            return;
        }
        auto& object = ensureRuntimeObject(objectName, className, "script_object");

        std::vector<std::string> chain;
        std::set<std::string> visited;
        std::string current = className;
        for (int depth = 0; depth < 16 && !current.empty() && visited.insert(current).second; ++depth) {
            chain.push_back(current);
            const auto base = classBases_.find(current);
            if (base == classBases_.end() || base->second == current) {
                break;
            }
            current = base->second;
        }

        for (auto it = chain.rbegin(); it != chain.rend(); ++it) {
            const auto defaults = classSlots_.find(*it);
            if (defaults == classSlots_.end()) {
                continue;
            }
            for (const auto& [key, value] : defaults->second) {
                if (value.kind == ScriptValueKind::Function || value.kind == ScriptValueKind::Method) {
                    continue;
                }
                object.fields.emplace(key, value);
            }
        }
    }

    ScriptValue lookupClassSlot(const std::string& className, const std::string& key) const
    {
        std::set<std::string> visited;
        std::string current = className;
        for (int depth = 0; depth < 16 && !current.empty() && visited.insert(current).second; ++depth) {
            const auto slots = classSlots_.find(current);
            if (slots != classSlots_.end()) {
                const auto slot = slots->second.find(key);
                if (slot != slots->second.end()) {
                    return slot->second;
                }
            }

            const auto base = classBases_.find(current);
            if (base == classBases_.end() || base->second == current) {
                break;
            }
            current = base->second;
        }
        return {};
    }

    const ScriptMethodBinding* findMethodBindingInHierarchy(
        const std::string& className,
        const std::string& slot) const
    {
        std::set<std::string> visited;
        std::string current = className;
        for (int depth = 0; depth < 16 && !current.empty() && visited.insert(current).second; ++depth) {
            const ScriptMethodBinding* method = findMethodBinding(methodBindings_, current, slot);
            if (method) {
                return method;
            }

            const auto base = classBases_.find(current);
            if (base == classBases_.end() || base->second == current) {
                break;
            }
            current = base->second;
        }
        return nullptr;
    }

    ScriptValue lookupRootSlot(const std::string& keyText) const
    {
        const auto rootSlot = rootFields_.find(keyText);
        if (rootSlot != rootFields_.end()) {
            return rootSlot->second;
        }
        const FunctionSlotBinding* functionSlot = findFunctionSlot(functionSlots_, keyText);
        if (functionSlot) {
            return functionValue(functionSlot->functionName, functionSlot->functionOrdinal);
        }
        if (hasClass(methodBindings_, keyText)) {
            return classValue(keyText);
        }
        if (registry_.find(keyText) || isValueHelperCall(keyText)) {
            return nativeFunctionValue(keyText);
        }
        if (isSquirrelValueMethod(keyText)) {
            return valueMethodValue(keyText, keyText);
        }
        if (keyText == "modMenu" || keyText == "modShop") {
            return nullValue();
        }
        if (keyText == "ms") {
            return objectValue("ms");
        }
        return {};
    }

    std::string rootObjectReceiverForValue(const ScriptValue& value) const
    {
        if (value.kind == ScriptValueKind::Object && rootObjects_.find(value.text) != rootObjects_.end()) {
            return value.text;
        }
        if (value.kind == ScriptValueKind::Table) {
            for (const auto& [rootName, rootValue] : rootFields_) {
                if (rootObjects_.find(rootName) != rootObjects_.end()
                    && rootValue.kind == ScriptValueKind::Table
                    && rootValue.tableId == value.tableId) {
                    return rootName;
                }
            }
        }
        return {};
    }

    ScriptValue lookupValue(const ScriptValue& target, const ScriptValue& key) const
    {
        const std::string keyText = slotKey(key);
        if (keyText.empty()) {
            return {};
        }

        if (target.kind == ScriptValueKind::Root) {
            return lookupRootSlot(keyText);
        }

        if (target.kind == ScriptValueKind::Object) {
            const auto object = runtimeObjects_.find(target.text);
            if (object != runtimeObjects_.end()) {
                const auto field = object->second.fields.find(keyText);
                if (field != object->second.fields.end()) {
                    return field->second;
                }
                if (!object->second.className.empty()) {
                    if (findMethodBindingInHierarchy(object->second.className, keyText)) {
                        return methodValue(keyText, target.text);
                    }
                    ScriptValue classSlot = lookupClassSlot(object->second.className, keyText);
                    if (isKnownValue(classSlot)) {
                        return classSlot;
                    }
                }
            }
            if (isUiObjectMethod(keyText)) {
                return uiMethodValue(keyText, target.text);
            }
            if (isMissionRuntimeMethod(keyText)) {
                return valueMethodValue(keyText, target.text);
            }
            return lookupRootSlot(keyText);
        }

        if (target.kind == ScriptValueKind::Class) {
            if (findMethodBinding(methodBindings_, target.text, keyText)) {
                return methodValue(keyText, target.text);
            }
            ScriptValue classSlot = lookupClassSlot(target.text, keyText);
            if (isKnownValue(classSlot)) {
                return classSlot;
            }
            return {};
        }

        if (target.kind == ScriptValueKind::Table) {
            const auto table = runtimeTables_.find(target.tableId);
            if (table != runtimeTables_.end()) {
                const auto field = table->second.fields.find(keyText);
                if (field != table->second.fields.end()) {
                    return field->second;
                }
            }
            return {};
        }

        if (target.kind == ScriptValueKind::SaveList) {
            if (keyText == "count" || keyText == "get") {
                return valueMethodValue(keyText, valueSummary(target));
            }
            return {};
        }

        if (target.kind == ScriptValueKind::ScenarioKeys) {
            if (keyText == "count") {
                return valueMethodValue(keyText, valueSummary(target));
            }
            return {};
        }

        if (target.kind == ScriptValueKind::SaveEntry) {
            if (keyText == "isActive") {
                return valueMethodValue(keyText, valueSummary(target));
            }
            return {};
        }

        if (target.kind == ScriptValueKind::Vector2) {
            if (keyText == "toPoint" || keyText == "toAbsPos") {
                return valueMethodValue(keyText, valueSummary(target));
            }
            return {};
        }

        if (target.kind == ScriptValueKind::Vector3 && keyText == "toVec3") {
            return valueMethodValue(keyText, valueSummary(target));
        }

        if (target.kind == ScriptValueKind::Quaternion && keyText == "toQuat") {
            return valueMethodValue(keyText, valueSummary(target));
        }

        if (target.kind == ScriptValueKind::Parameter && keyText == "getRequest") {
            return valueMethodValue(keyText, valueSummary(target));
        }

        if (target.kind == ScriptValueKind::Marker) {
            if (keyText == "_pos") {
                return vector3Value(valueSummary(target) + "._pos");
            }
            if (keyText == "_rot") {
                return quaternionValue(valueSummary(target) + "._rot");
            }
            return {};
        }

        if (target.kind == ScriptValueKind::Actor && isActorRuntimeMethod(keyText)) {
            return valueMethodValue(keyText, valueSummary(target));
        }

        if (target.kind == ScriptValueKind::Loader && isLoaderRuntimeMethod(keyText)) {
            return valueMethodValue(keyText, valueSummary(target));
        }

        if (target.kind == ScriptValueKind::PlaceParams && isPlaceParamsRuntimeMethod(keyText)) {
            return valueMethodValue(keyText, valueSummary(target));
        }

        if (isSquirrelValueMethod(keyText)) {
            return valueMethodValue(keyText, valueSummary(target));
        }
        if (registry_.find(keyText) || isValueHelperCall(keyText)) {
            return nativeFunctionValue(keyText, target.text);
        }
        return {};
    }

    ScriptValue canonicalizeRootSlotAssignment(const std::string& keyText, const ScriptValue& value)
    {
        if (value.kind != ScriptValueKind::Object) {
            return value;
        }

        const ScriptObjectBinding* binding = findObjectBinding(objectBindings_, keyText);
        if (!binding) {
            return value;
        }

        const auto object = runtimeObjects_.find(value.text);
        if (object == runtimeObjects_.end() || object->second.className != binding->className) {
            return value;
        }

        ensureRuntimeObject(keyText, binding->className, "script_object");
        if (value.text != keyText) {
            runtimeObjects_.erase(value.text);
        }
        return objectValue(keyText);
    }

    ScriptValue canonicalizeTableSlotAssignment(int tableId, const std::string& keyText, const ScriptValue& value)
    {
        if (value.kind != ScriptValueKind::Object || keyText.empty()) {
            return value;
        }

        std::string ownerObject;
        for (const auto& [objectName, object] : runtimeObjects_) {
            const auto sceneField = object.fields.find("_scenes");
            if (sceneField != object.fields.end() && sceneField->second.kind == ScriptValueKind::Table
                && sceneField->second.tableId == tableId) {
                ownerObject = objectName;
                break;
            }
        }
        if (ownerObject.empty()) {
            return value;
        }

        int sceneIndex = -1;
        try {
            size_t parsed = 0;
            sceneIndex = std::stoi(keyText, &parsed);
            if (parsed != keyText.size() || sceneIndex < 0) {
                return value;
            }
        } catch (...) {
            return value;
        }

        const std::string canonicalObject = ownerObject + "._scenes[" + std::to_string(sceneIndex) + "]";
        const auto source = runtimeObjects_.find(value.text);
        if (source == runtimeObjects_.end()) {
            return value;
        }

        const std::string className = source->second.className;
        if (className.empty()) {
            return value;
        }

        objectClasses_[canonicalObject] = className;
        RuntimeObject sourceObject = source->second;
        auto& canonical = ensureRuntimeObject(canonicalObject, className,
            sourceObject.runtimeType.empty() ? "script_object" : sourceObject.runtimeType);
        for (const auto& [field, fieldValue] : sourceObject.fields) {
            canonical.fields[field] = fieldValue;
        }

        if (value.text != canonicalObject) {
            runtimeObjects_.erase(value.text);
            objectClasses_.erase(value.text);
        }
        return objectValue(canonicalObject);
    }

    void assignSlot(const ScriptValue& target, const ScriptValue& key, const ScriptValue& value)
    {
        const std::string keyText = slotKey(key);
        if (keyText.empty()) {
            return;
        }

        if (target.kind == ScriptValueKind::Root) {
            rootFields_[keyText] = canonicalizeRootSlotAssignment(keyText, value);
            if (report_) {
                ++report_->rootSlotWrites;
            }
            return;
        }

        if (target.kind == ScriptValueKind::Class) {
            classSlots_[target.text][keyText] = value;
            if (report_) {
                ++report_->classSlotWrites;
            }
            return;
        }

        if (target.kind == ScriptValueKind::Object) {
            auto& object = ensureRuntimeObject(target.text, {}, {});
            object.fields[keyText] = value;
            if (report_) {
                ++report_->objectFieldWrites;
            }
            return;
        }

        if (target.kind == ScriptValueKind::Table) {
            runtimeTables_[target.tableId].fields[keyText] =
                canonicalizeTableSlotAssignment(target.tableId, keyText, value);
            if (report_) {
                ++report_->tableSlotWrites;
            }
            return;
        }
    }

    void recordServiceState(
        const std::string& service,
        const std::string& api,
        const std::string& action,
        const std::string& target,
        const std::string& value,
        const SqasmFunction& function,
        const SqasmInstruction& instruction,
        const std::string& evidence,
        std::string arguments = {})
    {
        catalog_.recordStateMutation({service, api, action, target, value, arguments});
        if (!report_) {
            return;
        }

        ++report_->serviceStateEventCount;
        if (hasDecodedArgumentPayload(arguments)) {
            ++report_->decodedServiceArguments;
        }
        if (service == "Save/Profile/Scenario Service" && api == "GetSaveList") {
            ++report_->saveServiceQueries;
        } else if (service == "Platform Service") {
            ++report_->platformStateQueries;
        } else if (service == "Audio Service") {
            ++report_->audioServiceCommands;
        } else if (service == "Scene And Stage Service") {
            ++report_->sceneServiceCommands;
        } else if (service == "UI And 2D Render Service" && action == "create_ui_object") {
            ++report_->uiObjectsTracked;
        } else if (service == "UI And 2D Render Service") {
            ++report_->uiServiceCommands;
        } else if (service == "Save/Profile/Scenario Service") {
            ++report_->valueStateQueries;
        }

        ScriptServiceStateEvent event;
        event.service = service;
        event.api = api;
        event.action = action;
        event.target = target;
        event.value = value;
        event.arguments = arguments;
        event.functionName = function.name;
        event.functionOrdinal = function.ordinal;
        event.sourceLine = instruction.sourceLine;
        event.pc = instruction.pc;
        event.evidence = evidence;
        report_->serviceStateEvents.push_back(event);
    }

    void recordServiceState(
        const std::string& service,
        const std::string& api,
        const std::string& action,
        const std::string& target,
        const std::string& value,
        const SqasmFunction& function,
        const SqasmCall& call,
        const std::string& evidence,
        std::string arguments = {})
    {
        catalog_.recordStateMutation({service, api, action, target, value, arguments});
        if (!report_) {
            return;
        }

        ++report_->serviceStateEventCount;
        if (hasDecodedArgumentPayload(arguments)) {
            ++report_->decodedServiceArguments;
        }
        if (service == "UI And 2D Render Service") {
            ++report_->uiServiceCommands;
        }

        ScriptServiceStateEvent event;
        event.service = service;
        event.api = api;
        event.action = action;
        event.target = target;
        event.value = value;
        event.arguments = arguments;
        event.functionName = function.name;
        event.functionOrdinal = function.ordinal;
        event.sourceLine = call.sourceLine;
        event.pc = call.pc;
        event.evidence = evidence;
        report_->serviceStateEvents.push_back(event);
    }

    ScriptValue makeCallReturn(
        const SqasmFunction& function,
        const SqasmInstruction& instruction,
        const ScriptValue& callable,
        const std::vector<ScriptValue>& arguments)
    {
        const std::string name = callable.text;
        if (name.empty()) {
            return {};
        }

        auto recordTypedReturn = [&](ScriptValue value) {
            if (report_) {
                ++report_->typedCallReturns;
            }
            return value;
        };

        const std::string argsText = argumentSummary(arguments);
        const std::string positionalArgsText = positionalArgumentSummary(arguments);

        auto readObjectField = [&](const std::string& objectName, const std::string& key) {
            return lookupValue(objectValue(objectName), stringValue(key));
        };
        auto writeObjectField = [&](const std::string& objectName, const std::string& key, const ScriptValue& value) {
            if (isConcreteReceiver(objectName)) {
                assignSlot(objectValue(objectName), stringValue(key), value);
            }
        };

        if (name == "float2" || name == "centerPos") {
            const std::string label = positionalArgsText.empty()
                ? name + "@" + std::to_string(instruction.pc)
                : name + "(" + positionalArgsText + ")";
            return recordTypedReturn(vector2Value(label));
        }
        if (name == "Vec3") {
            const std::string label = positionalArgsText.empty()
                ? name + "@" + std::to_string(instruction.pc)
                : name + "(" + positionalArgsText + ")";
            return recordTypedReturn(vector3Value(label));
        }
        if (name == "toPoint" || name == "toAbsPos" || name == "bl" || name == "tr") {
            if ((name == "bl" || name == "tr") && callable.kind == ScriptValueKind::UiMethod
                && isConcreteReceiver(callable.receiver)) {
                recordServiceState("UI And 2D Render Service", name, "ui_anchor_query", callable.receiver,
                    name + "(" + callable.receiver + ")", function, instruction,
                    "UI helper anchor query returns a vector used by layout code", argsText);
            }
            return recordTypedReturn(vector2Value(name + "(" + callable.receiver + ")"));
        }
        if (name == "toVec3") {
            return recordTypedReturn(vector3Value("toVec3(" + callable.receiver + ")"));
        }
        if (name == "toQuat") {
            return recordTypedReturn(quaternionValue("toQuat(" + callable.receiver + ")"));
        }
        if (name == "tofloat" || name == "tointeger") {
            double data = 0.0;
            if (!parseNumberText(callable.receiver, data)) {
                return {};
            }
            return name == "tofloat"
                ? recordTypedReturn(floatValue(data))
                : recordTypedReturn(intValue(static_cast<int>(data)));
        }
        if (name == "getRequest") {
            const std::string request = "mission_request:" + std::string(kFirstMissionEventScript);
            recordServiceState("Event/Quest/Flag Service", name, "mission_request",
                callable.receiver.empty() ? "This" : callable.receiver, request, function, instruction,
                "mission setupProcess asks the runtime request object for spawn marker context", argsText);
            return recordTypedReturn(missionRequestValue(request));
        }
        if (name == "GetSaveList") {
            recordServiceState("Save/Profile/Scenario Service", name, "query_empty_save_list", "save_list",
                options_.saveListEmpty ? "entries=0" : "entries=1", function, instruction,
                "runtime input scenario supplies save-list availability for title/menu branch execution");
            return recordTypedReturn(saveListValue());
        }
        if (name == "GetScenarioKeys") {
            recordServiceState("Save/Profile/Scenario Service", name, "query_scenario_keys", "scenario_keys",
                "entries=" + std::to_string(scenarioKeys().size()), function, instruction,
                "recovered title new-game flow queries scenario keys from ak3 _scenarios", argsText);
            return recordTypedReturn(scenarioKeysValue());
        }
        if (name == "GetCountActiveDLC") {
            recordServiceState("Platform Service", name, "active_dlc_count", "local_project", "0",
                function, instruction,
                "deterministic local project has no active DLC slots for new-game character count", argsText);
            return recordTypedReturn(intValue(0));
        }
        if (name == "IsSaveFull") {
            recordServiceState("Save/Profile/Scenario Service", name, "save_capacity_query", "save_profile",
                "false", function, instruction,
                "runtime profile is not full, so NewGameScene can start without overwrite confirmation", argsText);
            return recordTypedReturn(boolValue(false));
        }
        if (name == "SetDifficultyMode") {
            const std::string difficulty =
                arguments.empty() ? "unknown" : valueSummary(arguments.front());
            recordServiceState("Save/Profile/Scenario Service", name, "set_difficulty_mode", "save_profile",
                "difficulty=" + difficulty, function, instruction,
                "NewGameScene forwards selected difficulty to original game service", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "count") {
            if (callable.receiver.rfind("scenario_keys", 0) == 0) {
                recordServiceState("Save/Profile/Scenario Service", name, "scenario_key_count", callable.receiver,
                    std::to_string(scenarioKeys().size()), function, instruction,
                    "scenario key list exposes original ak3 _scenarios to NewGameScene.state0", argsText);
                return recordTypedReturn(intValue(static_cast<int>(scenarioKeys().size())));
            }
            if (isConcreteReceiver(callable.receiver)) {
                recordServiceState("Save/Profile/Scenario Service", name, "save_list_count", callable.receiver,
                    options_.saveListEmpty ? "0" : "1", function, instruction,
                    "runtime input scenario supplies save-list count for branch execution");
            }
            return recordTypedReturn(intValue(options_.saveListEmpty ? 0 : 1));
        }
        if (name == "get") {
            if (callable.receiver.rfind("scenario_keys", 0) == 0) {
                const std::string scenarioKey = scenarioKeyFromArguments(arguments);
                recordServiceState("Save/Profile/Scenario Service", name, "scenario_key_get", callable.receiver,
                    "scenario_key=" + scenarioKey, function, instruction,
                    "scenario key list returns the selected new-game player key", argsText);
                return recordTypedReturn(stringValue(scenarioKey));
            }
            if (isConcreteReceiver(callable.receiver)) {
                recordServiceState("Save/Profile/Scenario Service", name, "save_list_get", callable.receiver,
                    argsText.empty() ? "save_entry" : "save_entry " + argsText, function, instruction,
                    "branch-scanned typed save-list contract; entry shape is materialized without active save data",
                    argsText);
            }
            return recordTypedReturn(saveEntryValue("save_entry@" + std::to_string(instruction.pc)));
        }
        if (name == "isActive") {
            if (isConcreteReceiver(callable.receiver)) {
                recordServiceState("Save/Profile/Scenario Service", name, "save_entry_active", callable.receiver,
                    options_.saveListEmpty ? "false" : "true", function, instruction,
                    "runtime input scenario supplies save-entry activity for branch execution");
            }
            return recordTypedReturn(boolValue(!options_.saveListEmpty));
        }
        if (name == "IsFreeDemo" || name == "IsOverDemo" || name == "IsTrial") {
            recordServiceState("Platform Service", name, "platform_flag", "local_project", "false", function,
                instruction, "platform/demo state defaults to full local project mode");
            return recordTypedReturn(boolValue(false));
        }
        if (name == "CanShutdown") {
            recordServiceState("Platform Service", name, "shutdown_permission", "local_project",
                options_.canShutdown ? "true" : "false", function, instruction,
                "runtime input scenario supplies platform shutdown permission for title exit branch", argsText);
            return recordTypedReturn(boolValue(options_.canShutdown));
        }
        if (name == "GetDeltaTime") {
            recordServiceState("Script Service", name, "frame_delta_query", "runtime_clock", "0.0166667",
                function, instruction,
                "deterministic runtime clock delta used by recovered title/menu script state", argsText);
            return recordTypedReturn(floatValue(1.0 / 60.0));
        }
        if (callable.receiver == "gMenu"
            && (name == "isDecide" || name == "isMenuDown" || name == "isMenuUp" || name == "isCancel")) {
            bool pressed = options_.menuDecide;
            if (name == "isMenuDown") {
                pressed = options_.menuDown;
            } else if (name == "isMenuUp") {
                pressed = options_.menuUp;
            } else if (name == "isCancel") {
                pressed = false;
            }
            recordServiceState("Script Service", name, "menu_input_query", "gMenu",
                pressed ? "true" : "false", function, instruction,
                "runtime input scenario answers original gMenu input query", argsText);
            return recordTypedReturn(boolValue(pressed));
        }
        if (callable.receiver == "gMenu" && name == "savesIsEmpty") {
            recordServiceState("Script Service", name, "menu_save_state_query", "gMenu",
                options_.saveListEmpty ? "true" : "false", function, instruction,
                "runtime input scenario answers original title save availability query", argsText);
            return recordTypedReturn(boolValue(options_.saveListEmpty));
        }
        if (callable.receiver == "gMenu" && name == "continueDisabled") {
            recordServiceState("Script Service", name, "menu_continue_state_query", "gMenu",
                options_.continueDisabled ? "true" : "false", function, instruction,
                "runtime input scenario answers original title continue availability query", argsText);
            return recordTypedReturn(boolValue(options_.continueDisabled));
        }
        if (callable.receiver == "gMenu" && name == "setMission") {
            const std::string mission =
                arguments.empty() ? "unknown" : argumentValueText(arguments.front());
            recordServiceState("Save/Profile/Scenario Service", name, "set_current_mission", "gMenu",
                "mission=" + mission, function, instruction,
                "title continue/load branch forwards recovered save mission to gMenu", argsText);
            return recordTypedReturn(nullValue());
        }
        if (callable.receiver == "gMenu" && name == "setMissionKey") {
            const std::string missionKey =
                arguments.empty() ? "unknown" : valueSummary(arguments.front());
            recordServiceState("Save/Profile/Scenario Service", name, "set_current_mission_key", "gMenu",
                "mission_key=" + missionKey, function, instruction,
                "new-game branch forwards selected scenario key to gMenu", argsText);
            return recordTypedReturn(nullValue());
        }
        if (callable.receiver == "gMenu" && name == "clearMission") {
            recordServiceState("Save/Profile/Scenario Service", name, "clear_menu_mission", "gMenu",
                "mission=null; mission_key=null", function, instruction,
                "startGame4Menu clears pending gMenu mission slots after StartGame", argsText);
            return recordTypedReturn(nullValue());
        }
        if (callable.receiver == "gMenu" && name == "fadeOut") {
            recordServiceState("Scene And Stage Service", name, "fade_out", "screen",
                argsText.empty() ? "fade_out" : argsText, function, instruction,
                "ModuleTitle transition branch queries gMenu fadeOut before startGame4Menu", argsText);
            return recordTypedReturn(boolValue(false));
        }
        if (callable.receiver == "gMenu" && name == "startGame4Menu") {
            recordServiceState("Script Service", name, "start_game_from_menu", "gMenu",
                "startGame4Menu", function, instruction,
                "ModuleTitle transition branch starts gameplay through original gMenu API", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "MakeNewGame") {
            const std::string scenarioKey = scenarioKeyFromArguments(arguments);
            const std::string mission = missionLabelForScenarioKey(scenarioKey);
            recordServiceState("Save/Profile/Scenario Service", name, "make_new_game", "save_profile",
                "scenario_key=" + scenarioKey + "; mission=" + mission, function, instruction,
                "startGame4Menu calls MakeNewGame with gMenu.missionKey and consumes its mission return value",
                argsText);
            return recordTypedReturn(missionValue(mission));
        }
        if (name == "StartGame") {
            const std::string mission =
                arguments.empty() ? "unknown" : argumentValueText(arguments.front());
            const bool newGame =
                arguments.size() > 1 && arguments[1].kind == ScriptValueKind::Bool && arguments[1].boolValue;
            recordServiceState("Save/Profile/Scenario Service", name, "start_game", "save_profile",
                "mission=" + mission + "; new_game=" + (newGame ? "true" : "false"), function, instruction,
                "startGame4Menu passes the generated mission and new-game flag to original StartGame", argsText);
            recordServiceState("Scene And Stage Service", name, "queue_scene_stage_load", kFirstMissionScript,
                "mission=" + mission + "; mission_script=" + kFirstMissionScript + "; stage=" + kFirstMissionStage
                    + "; rail_camera=" + kFirstMissionRailCamera,
                function, instruction,
                "StartGame queues the first mission script and setupProcess stage resources proven by the oracle",
                argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "Loader") {
            const std::string missionScript =
                arguments.empty() ? kFirstMissionEventScript : argumentValueText(arguments.front());
            const std::string loader = "loader:" + missionScript;
            recordServiceState("Scene And Stage Service", name, "create_loader", loader,
                "loader=" + loader + "; event_script=" + missionScript, function, instruction,
                "first mission setupProcess creates the loader for the original mission script", argsText);
            return recordTypedReturn(loaderValue(loader));
        }
        if (name == "LoadStage") {
            const std::string stage = arguments.empty() ? kFirstMissionStage : argumentValueText(arguments.front());
            const std::string blocking = arguments.size() > 1 ? valueSummary(arguments[1]) : "unknown";
            recordServiceState("Scene And Stage Service", name, "load_stage", stage,
                "stage=" + stage + "; blocking=" + blocking, function, instruction,
                "first mission setupProcess loads the stage resource before event setup", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "LoadEventsScriptViaMission") {
            const std::string eventScript =
                arguments.empty() ? kFirstMissionEventScript : argumentValueText(arguments.front());
            recordServiceState("Scene And Stage Service", name, "load_events_script_via_mission",
                eventScript,
                "event_script=" + eventScript + "; mission_script=" + kFirstMissionScript,
                function, instruction,
                "first mission setupProcess gates later setup on event script load success", argsText);
            return recordTypedReturn(boolValue(true));
        }
        if (name == "ClearCurrentQuest") {
            recordServiceState("Event/Quest/Flag Service", name, "clear_current_quest", "quest_state",
                "cleared", function, instruction,
                "mission setup clears current quest before calling setup events", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "CallSetupEvents") {
            const std::string changed = arguments.empty() ? "unknown" : valueSummary(arguments.front());
            recordServiceState("Scene And Stage Service", name, "call_setup_events",
                kFirstMissionEventObject, "quest_changed=" + changed, function, instruction,
                "mission setup calls recovered event setup after loading event script", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "GetMarkerFromRequest") {
            recordServiceState("Event/Quest/Flag Service", name, "marker_from_request", "eventMap",
                std::string("marker=") + kFirstMissionMarker + "; request="
                    + (arguments.size() > 1 ? valueSummary(arguments[1]) : "unknown"),
                function, instruction,
                "mission setup resolves player spawn marker from eventMap and mission request", argsText);
            return recordTypedReturn(markerValue(kFirstMissionMarker));
        }
        if (name == "QuatToRotYDegree") {
            recordServiceState("Script Service", name, "quat_to_rot_y_degree", "marker_rotation",
                arguments.empty() ? "rot_y=0" : "rot_y=0; quat=" + valueSummary(arguments.front()),
                function, instruction,
                "mission setup converts marker quaternion to player Y rotation", argsText);
            return recordTypedReturn(floatValue(0.0));
        }
        if (name == "PushPlayerChara") {
            const std::string chara = arguments.empty() ? kFirstMissionPlayer : argumentValueText(arguments.front());
            const std::string pos = arguments.size() > 1 ? valueSummary(arguments[1]) : kFirstMissionPlayerPos;
            const std::string rotY = arguments.size() > 2 ? valueSummary(arguments[2]) : "0";
            const std::string actor = "actor:player:" + chara;
            recordServiceState("Actor And Task Service", name, "push_player_chara", actor,
                "chara=" + chara + "; pos=" + pos + "; rot_y=" + rotY, function, instruction,
                "mission setup pushes the playable character actor from original spawn marker", argsText);
            return recordTypedReturn(actorValue(actor));
        }
        if (name == "setWaitForLanding") {
            const std::string enabled = arguments.empty() ? "unknown" : valueSummary(arguments.front());
            recordServiceState("Actor And Task Service", name, "set_wait_for_landing", callable.receiver,
                "enabled=" + enabled, function, instruction,
                "mission setup configures the spawned player actor landing wait flag", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "fillHealProgress") {
            recordServiceState("Actor And Task Service", name, "fill_heal_progress", callable.receiver,
                "filled", function, instruction,
                "mission setup fills the spawned player actor heal progress", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "setArmed") {
            const std::string armed = arguments.empty() ? "unknown" : valueSummary(arguments.front());
            recordServiceState("Actor And Task Service", name, "set_armed", callable.receiver,
                "armed=" + armed, function, instruction,
                "mission setup arms the spawned player actor", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "playEffect") {
            const std::string effect = arguments.empty() ? "unknown" : valueSummary(arguments.front());
            const std::string pos = arguments.size() > 1 ? valueSummary(arguments[1]) : "unknown";
            recordServiceState("Actor And Task Service", name, "play_effect", callable.receiver,
                "effect=" + effect + "; pos=" + pos, function, instruction,
                "mission setup plays spawn effect on the player actor", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "PushTaskGameCamera") {
            recordServiceState("Camera Service", name, "push_task_game_camera", "camera_task",
                "pushed", function, instruction,
                "mission setup pushes the game camera task before checkpoint and rail camera setup", argsText);
            return recordTypedReturn(cameraTaskValue("camera:game"));
        }
        if (name == "SetCheckPoint") {
            const std::string checkpoint = arguments.empty() ? kFirstMissionPlayerPos : valueSummary(arguments.front());
            recordServiceState("Event/Quest/Flag Service", name, "set_checkpoint", "checkpoint",
                "checkpoint=" + checkpoint, function, instruction,
                "mission setup writes checkpoint from player spawn position", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "LoadRailCamera") {
            const std::string railCamera =
                arguments.empty() ? kFirstMissionRailCamera : argumentValueText(arguments.front());
            recordServiceState("Camera Service", name, "load_rail_camera", "rail_camera",
                "rail_camera=" + railCamera, function, instruction,
                "mission setup loads rail camera resource for the first stage", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "SetEnableRailCamera") {
            const std::string enabled = arguments.empty() ? "unknown" : valueSummary(arguments.front());
            recordServiceState("Camera Service", name, "set_enable_rail_camera", "rail_camera",
                "enabled=" + enabled, function, instruction,
                "mission setup enables rail camera after loading the resource", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "SetEnableAutoCameraAdjust") {
            const std::string enabled = arguments.empty() ? "unknown" : valueSummary(arguments.front());
            recordServiceState("Camera Service", name, "set_enable_auto_camera_adjust", "game_camera",
                "enabled=" + enabled, function, instruction,
                "mission setup disables automatic camera adjust for the first scripted spawn", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "SetDefaultCameraState") {
            recordServiceState("Camera Service", name, "set_default_camera_state", "game_camera",
                callable.receiver.empty() ? kFirstMissionEventObject : callable.receiver,
                function, instruction,
                "mission setup applies default camera state through the event object", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "LoadCharaPlace") {
            const std::string place = arguments.empty() ? "" : argumentValueText(arguments.front());
            recordServiceState("Scene And Stage Service", name, "load_chara_place", "chara_place",
                "place=" + place, function, instruction,
                "mission setup loads character placement data for the current stage", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "FadeOutBGM") {
            const std::string duration = arguments.empty() ? "unknown" : valueSummary(arguments.front());
            recordServiceState("Audio Service", name, "fade_out_bgm", "bgm",
                "duration=" + duration, function, instruction,
                "mission setup fades out title/menu BGM before stage gameplay", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "GetPlaceParams") {
            recordServiceState("Scene And Stage Service", name, "get_place_params", "place_params",
                "place_params=current", function, instruction,
                "mission setup fetches place parameters for current location UI/world state", argsText);
            return recordTypedReturn(placeParamsValue("place_params:current"));
        }
        if (name == "setLabel") {
            const std::string label = arguments.empty() ? "unknown" : argumentValueText(arguments.front());
            recordServiceState("Scene And Stage Service", name, "set_place_label", callable.receiver,
                "label=" + label, function, instruction,
                "mission setup labels the active place params with original text id", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "setEnabledRetWorld") {
            const std::string enabled = arguments.empty() ? "unknown" : valueSummary(arguments.front());
            recordServiceState("Scene And Stage Service", name, "set_enabled_ret_world", callable.receiver,
                "enabled=" + enabled, function, instruction,
                "mission setup enables return-to-world state on place params", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "setSlaveDisp") {
            const std::string mode = arguments.empty() ? "unknown" : valueSummary(arguments.front());
            recordServiceState("Scene And Stage Service", name, "set_slave_disp", callable.receiver,
                "mode=" + mode, function, instruction,
                "mission setup applies slave display mode on place params", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "end") {
            recordServiceState("Scene And Stage Service", name, "loader_end", callable.receiver,
                "ended", function, instruction,
                "mission setup finalizes the loader after stage/event/player/camera setup", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "checkFall") {
            recordServiceState("Event/Quest/Flag Service", name, "check_fall", "mission_state",
                "checked", function, instruction,
                "mission main tick checks player fall state after setupProcess", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "MenuObject") {
            const std::string objectName = "ui.MenuObject@" + function.name + ":" + std::to_string(instruction.pc);
            objectClasses_[objectName] = "MenuObject";
            ensureRuntimeObject(objectName, "MenuObject", "ui_object");
            materializeClassDefaults(objectName, "MenuObject");
            recordServiceState("UI And 2D Render Service", name, "create_ui_object", objectName, "MenuObject",
                function, instruction, "MenuObject returns a tracked UI helper object");
            return recordTypedReturn(objectValue(objectName));
        }
        if (callable.kind == ScriptValueKind::Method && name == "setSelectCursor"
            && isConcreteReceiver(callable.receiver)) {
            if (!arguments.empty()) {
                writeObjectField(callable.receiver, "_sel", arguments[0]);
            }
            if (arguments.size() > 1) {
                writeObjectField(callable.receiver, "_elemCount", arguments[1]);
            }
            recordServiceState("Script Service", name, "menu_cursor_set", callable.receiver,
                argsText.empty() ? "select_cursor" : argsText, function, instruction,
                "ScrollWindow.setSelectCursor applies recovered Squirrel arguments to runtime object state",
                argsText);
            return recordTypedReturn(nullValue());
        }
        if (callable.kind == ScriptValueKind::Method && name == "selectCursorY"
            && isConcreteReceiver(callable.receiver)) {
            if (!arguments.empty()) {
                writeObjectField(callable.receiver, "_elemCount", arguments[0]);
            }
            ScriptValue selected = intValue(options_.menuSelectedIndex);
            writeObjectField(callable.receiver, "_sel", selected);
            recordServiceState("Script Service", name, "menu_cursor_select", callable.receiver,
                valueSummary(selected), function, instruction,
                "ScrollWindow.selectCursorY returns the runtime input scenario selected row", argsText);
            return recordTypedReturn(selected);
        }
        if (callable.kind == ScriptValueKind::Method && name == "selectCursorX"
            && isConcreteReceiver(callable.receiver)) {
            if (!arguments.empty()) {
                writeObjectField(callable.receiver, "_elemCount", arguments[0]);
            }
            ScriptValue selected = intValue(0);
            writeObjectField(callable.receiver, "_sel", selected);
            recordServiceState("Script Service", name, "menu_cursor_select_x", callable.receiver,
                valueSummary(selected), function, instruction,
                "NewGameScene.selectCursorX returns deterministic first player selection for scenario boot",
                argsText);
            return recordTypedReturn(selected);
        }
        if (callable.kind == ScriptValueKind::Method && name == "getDrawEnd"
            && isConcreteReceiver(callable.receiver)) {
            ScriptValue elemCount = readObjectField(callable.receiver, "_elemCount");
            ScriptValue drawCount = readObjectField(callable.receiver, "_drawCount");
            ScriptValue drawBegin = readObjectField(callable.receiver, "_drawBegin");
            double elem = 0.0;
            double draw = 0.0;
            double begin = 0.0;
            ScriptValue end = elemCount;
            if (numericValue(elemCount, elem) && numericValue(drawCount, draw) && numericValue(drawBegin, begin)) {
                end = intValue(static_cast<int>(std::min(elem, begin + draw)));
            }
            return recordTypedReturn(end);
        }
        if (callable.kind == ScriptValueKind::Method && name == "main" && isConcreteReceiver(callable.receiver)) {
            ScriptValue nextState = lookupValue(objectValue(callable.receiver), stringValue("_nextState"));
            if (!isKnownValue(nextState)) {
                nextState = intValue(0);
            }
            return recordTypedReturn(nextState);
        }
        if (hasClass(methodBindings_, name) || callable.kind == ScriptValueKind::Class) {
            const std::string className = callable.kind == ScriptValueKind::Class ? callable.text : name;
            const std::string objectName = "script." + className + "@" + function.name + ":"
                + std::to_string(instruction.pc);
            objectClasses_[objectName] = className;
            ensureRuntimeObject(objectName, className, "script_object");
            materializeClassDefaults(objectName, className);
            return recordTypedReturn(objectValue(objectName));
        }
        if (callable.kind == ScriptValueKind::UiMethod) {
            if (report_) {
                ++report_->uiObjectMutations;
            }
            if (name != "init" && isConcreteReceiver(callable.receiver)) {
                std::string value = "returns_receiver";
                if (name == "setParent" && !arguments.empty()) {
                    value = "parent=" + valueSummary(arguments.front());
                } else if (name == "setSelectCursor") {
                    value = argsText.empty() ? "select_cursor" : argsText;
                } else if (!argsText.empty()) {
                    value = argsText;
                }
                recordServiceState("UI And 2D Render Service", name, "ui_method_call", callable.receiver,
                    value, function, instruction,
                    "UI helper method preserves receiver identity with decoded Squirrel call arguments",
                    argsText);
            }
            return recordTypedReturn(objectValue(callable.receiver));
        }
        if (name == "FadeIn") {
            const std::string value = arguments.empty() ? "duration=unknown" : "duration=" + valueSummary(arguments[0])
                + (arguments.size() > 1 ? "; blend=" + valueSummary(arguments[1]) : std::string());
            recordServiceState("Scene And Stage Service", name, "fade_in", "screen", value,
                function, instruction, "FadeIn side effect reached through ModuleTitle.main boot edge", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "PlayBGM") {
            const std::string value = arguments.empty() ? "bgm_id=unknown" : "bgm_id=" + valueSummary(arguments[0]);
            recordServiceState("Audio Service", name, "play_bgm", "bgm", value, function, instruction,
                "PlayBGM side effect reached through ModuleTitle.main boot edge", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "PlaySE") {
            const std::string value = arguments.empty() ? "se_id=unknown" : "se_id=" + valueSummary(arguments[0]);
            recordServiceState("Audio Service", name, "play_se", "se", value, function, instruction,
                "PlaySE side effect reached through recovered title/menu branch", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "LoadAutoSave") {
            recordServiceState("Save/Profile/Scenario Service", name, "load_auto_save", "save_profile",
                options_.saveListEmpty ? "false" : "true", function, instruction,
                "runtime input scenario supplies autosave load outcome for continue branch", argsText);
            return recordTypedReturn(boolValue(!options_.saveListEmpty));
        }
        if (name == "ShutdownGame") {
            recordServiceState("Platform Service", name, "shutdown_game", "local_project", "requested",
                function, instruction, "title exit branch reached original ShutdownGame call", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "renderHorizontal") {
            recordServiceState("UI And 2D Render Service", name, "ui_render_method", callable.receiver,
                argsText.empty() ? "arguments_empty" : argsText, function, instruction,
                "UI render helper reached with decoded Squirrel call arguments", argsText);
            return recordTypedReturn(nullValue());
        }
        if (name == "stateInit") {
            recordServiceState("Script Service", name, "module_state_init", "ModuleTitle", "pending_state_dispatch",
                function, instruction, "stateInit side effect reached through ModuleTitle.main boot edge");
            return recordTypedReturn(nullValue());
        }
        if (name == "init" || name == "setParent" || name == "setSelectCursor"
            || name == "setFadeIn" || name == "resetAnim" || name == "move") {
            return recordTypedReturn(nullValue());
        }
        return {};
    }

    SynchronousCallResult executeScriptCallableForReturn(
        const ScriptValue& callable,
        const std::vector<ScriptValue>& arguments,
        const SqasmFunction& callerFunction,
        const SqasmInstruction& instruction,
        const std::string& ownerObject,
        int depth)
    {
        const std::string name = callable.text;
        if (name.empty()) {
            return {};
        }
        if (callable.kind == ScriptValueKind::Method
            && (name == "selectCursorY" || name == "selectCursorX" || name == "setSelectCursor"
                || name == "getDrawEnd" || name == "setFadeIn" || name == "resetAnim" || name == "move")) {
            return {};
        }

        SqasmCall syntheticCall;
        syntheticCall.name = name;
        syntheticCall.sourceLine = instruction.sourceLine;
        syntheticCall.pc = instruction.pc;

        if (callable.kind != ScriptValueKind::Method || callable.receiver.empty()) {
            return {};
        }

        const std::string receiver = callable.receiver;
        const auto objectClass = objectClasses_.find(receiver);
        if (objectClass != objectClasses_.end()) {
            if (!isReturnSynchronizedScriptClass(objectClass->second)) {
                return {};
            }
            const ScriptMethodBinding* method = findMethodBindingInHierarchy(objectClass->second, name);
            if (method) {
                const std::string category =
                    method->ownerClass == objectClass->second ? "script_object_method_sync_return"
                                                              : "script_inherited_object_method_sync_return";
                return {true, executeMethod(*method, receiver, method->ownerClass, depth + 1, category, arguments)};
            }
        }

        if (hasClass(methodBindings_, receiver)) {
            if (!isReturnSynchronizedScriptClass(receiver)) {
                return {};
            }
            const ScriptMethodBinding* method = findMethodBinding(methodBindings_, receiver, name);
            if (method) {
                const std::string targetObject = ownerObject.empty() ? receiver : ownerObject;
                return {true, executeMethod(*method, targetObject, receiver, depth + 1,
                    "script_super_or_class_method_sync_return", arguments)};
            }
        }

        recordEvent({"script_sync_return_unresolved", callerFunction.name, callerFunction.ordinal, {}, {},
            name, receiver, "script_call_return", {}, instruction.sourceLine, instruction.pc,
            "callsite needed a script return value but no script binding was recovered for receiver"});
        return {};
    }

    BytecodeStateResult executeBytecodeState(
        const SqasmFunction& function,
        const std::string& ownerObject,
        const std::string& ownerClass,
        int depth,
        const std::vector<ScriptValue>& arguments = {})
    {
        BytecodeStateResult result;
        if (report_) {
            ++report_->bytecodeStateFunctions;
        }
        std::map<int, size_t> instructionIndexByPc;
        for (size_t i = 0; i < function.instructions.size(); ++i) {
            instructionIndexByPc[function.instructions[i].pc] = i;
        }
        const size_t registerCount =
            static_cast<size_t>((function.stack > 0 ? function.stack : 0) + 16);
        std::vector<ScriptValue> registers(registerCount, unknownValue());
        auto setRegister = [&](int index, ScriptValue value) {
            if (index < 0 || static_cast<size_t>(index) >= registers.size()) {
                return;
            }
            registers[static_cast<size_t>(index)] = std::move(value);
        };
        auto getRegister = [&](int index) -> ScriptValue {
            if (index < 0 || static_cast<size_t>(index) >= registers.size()) {
                return {};
            }
            return registers[static_cast<size_t>(index)];
        };
        auto getCallArguments = [&](int receiverReg, int rawArgumentCount) {
            std::vector<ScriptValue> arguments;
            const int actualArgumentCount = rawArgumentCount > 0 ? rawArgumentCount - 1 : 0;
            arguments.reserve(static_cast<size_t>(actualArgumentCount));
            for (int n = 0; n < actualArgumentCount; ++n) {
                arguments.push_back(getRegister(receiverReg + 1 + n));
            }
            return arguments;
        };
        auto jumpTargetPc = [](const SqasmInstruction& instruction) {
            if (instruction.target >= 0) {
                return instruction.target;
            }
            return instruction.pc + argValue(instruction, "a1", 0) + 1;
        };
        auto jumpTo = [&](int targetPc, size_t& instructionIndex) {
            const auto found = instructionIndexByPc.find(targetPc);
            if (found == instructionIndexByPc.end()) {
                instructionIndex = function.instructions.size();
                return;
            }
            instructionIndex = found->second;
        };
        auto recordUnknownBranch = [&](const SqasmInstruction& instruction, const ScriptValue& condition) {
            result.controlFlowUnknown = true;
            recordEvent({"control_flow_unknown", function.name, function.ordinal, ownerObject, ownerClass,
                instruction.op, describeValue(condition), "bytecode_control_flow", {}, instruction.sourceLine,
                instruction.pc, "branch condition is not concrete yet; following fallthrough path only"});
        };
        auto foreachItems = [&](const ScriptValue& container) {
            std::vector<std::pair<std::string, ScriptValue>> items;
            if (container.kind == ScriptValueKind::Table) {
                const auto table = runtimeTables_.find(container.tableId);
                if (table != runtimeTables_.end()) {
                    for (const auto& field : table->second.fields) {
                        items.push_back(field);
                    }
                }
            } else if (container.kind == ScriptValueKind::Object) {
                const auto object = runtimeObjects_.find(container.text);
                if (object != runtimeObjects_.end()) {
                    for (const auto& field : object->second.fields) {
                        items.push_back(field);
                    }
                }
            }
            return items;
        };
        auto foreachExitIndex = [&](size_t foreachIndex) {
            const int foreachPc = function.instructions[foreachIndex].pc;
            for (size_t n = foreachIndex + 1; n < function.instructions.size(); ++n) {
                const auto& candidate = function.instructions[n];
                if (candidate.op == "_OP_JMP" && jumpTargetPc(candidate) == foreachPc) {
                    return n + 1;
                }
            }
            return function.instructions.size();
        };

        if (ownerObject.empty()) {
            setRegister(0, rootValue());
        } else {
            ensureRuntimeObject(ownerObject, ownerClass, ownerClass.empty() ? "script_object" : "script_object");
            setRegister(0, objectValue(ownerObject));
        }
        for (size_t i = 1; i < function.parameters.size(); ++i) {
            const size_t argumentIndex = i - 1;
            setRegister(static_cast<int>(i), argumentIndex < arguments.size()
                ? arguments[argumentIndex]
                : parameterValue(function.parameters[i]));
        }

        size_t i = 0;
        std::map<int, size_t> foreachCursorByPc;
        std::map<int, int> callPcByCallableRegister;
        const size_t maxInstructionExecutions =
            std::max<size_t>(512, function.instructions.size() * 16);
        while (i < function.instructions.size()) {
            if (static_cast<size_t>(result.executedInstructions) >= maxInstructionExecutions) {
                if (report_) {
                    report_->truncated = true;
                }
                recordEvent({"bytecode_state_truncated", function.name, function.ordinal, ownerObject, ownerClass,
                    {}, {}, "bytecode_control_flow", {}, -1, -1,
                    "instruction execution limit reached while following bytecode control flow"});
                break;
            }
            const auto& instruction = function.instructions[i];
            ++result.executedInstructions;

            const int a0 = argValue(instruction, "a0", -1);
            const int a1 = argValue(instruction, "a1", -1);
            const int a2 = argValue(instruction, "a2", -1);
            const int a3 = argValue(instruction, "a3", -1);

            if (instruction.op == "_OP_JMP") {
                jumpTo(jumpTargetPc(instruction), i);
                continue;
            }
            if (instruction.op == "_OP_JZ" || instruction.op == "_OP_JNZ") {
                const ScriptValue condition = getRegister(a0);
                const Truthiness truth = valueTruthiness(condition);
                if (truth == Truthiness::Unknown) {
                    recordUnknownBranch(instruction, condition);
                    ++i;
                    continue;
                }
                const bool shouldJump = instruction.op == "_OP_JZ"
                    ? truth == Truthiness::False
                    : truth == Truthiness::True;
                if (shouldJump) {
                    jumpTo(jumpTargetPc(instruction), i);
                } else {
                    ++i;
                }
                continue;
            }
            if (instruction.op == "_OP_RETURN") {
                if (a0 == 1) {
                    result.returnValue = getRegister(a1);
                }
                break;
            }
            if (instruction.op == "_OP_FOREACH") {
                const auto items = foreachItems(getRegister(a0));
                size_t& cursor = foreachCursorByPc[instruction.pc];
                if (cursor >= items.size()) {
                    foreachCursorByPc.erase(instruction.pc);
                    i = foreachExitIndex(i);
                    continue;
                }
                const auto& item = items[cursor++];
                setRegister(a1, intValue(static_cast<int>(cursor)));
                setRegister(a2, tableKeyValue(item.first));
                setRegister(a2 + 1, item.second);
                ++i;
                continue;
            }
            if (instruction.op == "_OP_POSTFOREACH") {
                ++i;
                continue;
            }

            if (instruction.op == "_OP_LOAD") {
                setRegister(a0, stringValue(literalValueByIndex(function, a1)));
            } else if (instruction.op == "_OP_DLOAD") {
                setRegister(a0, stringValue(literalValueByIndex(function, a1)));
                setRegister(a2, stringValue(literalValueByIndex(function, a3)));
            } else if (instruction.op == "_OP_LOADINT") {
                setRegister(a0, intValue(a1));
            } else if (instruction.op == "_OP_LOADFLOAT") {
                float data = 0.0F;
                std::memcpy(&data, &a1, sizeof(float));
                setRegister(a0, floatValue(data));
            } else if (instruction.op == "_OP_LOADBOOL") {
                setRegister(a0, boolValue(a1 != 0));
            } else if (instruction.op == "_OP_LOADNULLS") {
                for (int n = 0; n < a1; ++n) {
                    setRegister(a0 + n, nullValue());
                }
            } else if (instruction.op == "_OP_LOADROOTTABLE") {
                setRegister(a0, rootValue());
            } else if (instruction.op == "_OP_CLONE") {
                setRegister(a0, getRegister(a1));
            } else if (instruction.op == "_OP_MOVE") {
                setRegister(a0, getRegister(a1));
            } else if (instruction.op == "_OP_DMOVE") {
                setRegister(a0, getRegister(a1));
                setRegister(a2, getRegister(a3));
            } else if (instruction.op == "_OP_ARITH") {
                setRegister(a0, arithmeticValue(getRegister(a2), getRegister(a1), a3));
            } else if (instruction.op == "_OP_EQ") {
                const Truthiness truth = equalityTruthiness(getRegister(a2), getRegister(a1));
                if (truth == Truthiness::Unknown) {
                    setRegister(a0, expressionValue(valueSummary(getRegister(a2)) + " == "
                        + valueSummary(getRegister(a1))));
                } else {
                    setRegister(a0, boolValue(truth == Truthiness::True));
                }
            } else if (instruction.op == "_OP_NE") {
                const Truthiness truth = equalityTruthiness(getRegister(a2), getRegister(a1));
                if (truth == Truthiness::Unknown) {
                    setRegister(a0, expressionValue(valueSummary(getRegister(a2)) + " != "
                        + valueSummary(getRegister(a1))));
                } else {
                    setRegister(a0, boolValue(truth == Truthiness::False));
                }
            } else if (instruction.op == "_OP_NOT") {
                const Truthiness truth = valueTruthiness(getRegister(a1));
                if (truth == Truthiness::Unknown) {
                    setRegister(a0, expressionValue("!(" + valueSummary(getRegister(a1)) + ")"));
                } else {
                    setRegister(a0, boolValue(truth == Truthiness::False));
                }
            } else if (instruction.op == "_OP_CMP") {
                double lhs = 0.0;
                double rhs = 0.0;
                if (numericValue(getRegister(a2), lhs) && numericValue(getRegister(a1), rhs)) {
                    setRegister(a0, boolValue(lhs > rhs));
                } else {
                    setRegister(a0, expressionValue(valueSummary(getRegister(a2)) + " > "
                        + valueSummary(getRegister(a1))));
                }
            } else if (instruction.op == "_OP_EXISTS") {
                const ScriptValue table = getRegister(a1);
                const std::string key = slotKey(getRegister(a2));
                bool exists = false;
                if (table.kind == ScriptValueKind::Table) {
                    const auto found = runtimeTables_.find(table.tableId);
                    exists = found != runtimeTables_.end() && found->second.fields.find(key) != found->second.fields.end();
                } else if (table.kind == ScriptValueKind::Object) {
                    const auto found = runtimeObjects_.find(table.text);
                    exists = found != runtimeObjects_.end() && found->second.fields.find(key) != found->second.fields.end();
                }
                setRegister(a0, boolValue(exists));
            } else if (instruction.op == "_OP_NEWTABLE" || instruction.op == "_OP_NEWARRAY") {
                const int tableId = nextTableId_++;
                runtimeTables_[tableId] = {};
                setRegister(a0, tableValue(tableId));
            } else if (instruction.op == "_OP_CLOSURE") {
                if (!instruction.functionRefs.empty()) {
                    const auto& ref = instruction.functionRefs.front();
                    setRegister(a0, functionValue(ref.value, ref.index));
                }
            } else if (instruction.op == "_OP_CLASS") {
                const std::string className = classNameBefore(function.instructions, i);
                std::string baseClass;
                const ScriptValue base = getRegister(a1);
                if (base.kind == ScriptValueKind::Class || base.kind == ScriptValueKind::String) {
                    baseClass = base.text;
                }
                if (!className.empty()) {
                    if (!baseClass.empty()) {
                        classBases_[className] = baseClass;
                    }
                    setRegister(a0, classValue(className));
                }
            } else if (instruction.op == "_OP_GETK") {
                setRegister(a0, lookupValue(getRegister(a2), stringValue(literalValueByIndex(function, a1))));
            } else if (instruction.op == "_OP_GET") {
                setRegister(a0, lookupValue(getRegister(a1), getRegister(a2)));
            } else if (instruction.op == "_OP_SET") {
                assignSlot(getRegister(a1), getRegister(a2), getRegister(a3));
                setRegister(a0, getRegister(a3));
            } else if (instruction.op == "_OP_INCL") {
                double data = 0.0;
                if (numericValue(getRegister(a1), data)) {
                    const int delta = signedBytecodeDelta(a3);
                    const double next = data + static_cast<double>(delta);
                    setRegister(a0, floatValue(next));
                    setRegister(a1, floatValue(next));
                }
            } else if (instruction.op == "_OP_PINC" || instruction.op == "_OP_PINCL") {
                const ScriptValue target = getRegister(a0);
                const ScriptValue key = getRegister(a2);
                const ScriptValue current = lookupValue(target, key);
                double data = 0.0;
                if (numericValue(current, data)) {
                    const int delta = signedBytecodeDelta(a3);
                    assignSlot(target, key, floatValue(data + static_cast<double>(delta)));
                }
            } else if (instruction.op == "_OP_NEWSLOT" || instruction.op == "_OP_NEWSLOTA") {
                assignSlot(getRegister(a1), getRegister(a2), getRegister(a3));
            } else if (instruction.op == "_OP_PREPCALL" || instruction.op == "_OP_PREPCALLK") {
                result.executedCallPcs.insert(instruction.pc);
                callPcByCallableRegister[a0] = instruction.pc;
                const ScriptValue target = getRegister(a2);
                const ScriptValue key =
                    instruction.op == "_OP_PREPCALLK" ? stringValue(literalValueByIndex(function, a1))
                                                       : getRegister(a1);
                const std::string keyText = slotKey(key);
                const std::string rootObjectReceiver = rootObjectReceiverForValue(target);
                ScriptValue callable = lookupValue(target, key);
                if (!isKnownValue(callable)) {
                    if (registry_.find(keyText) || isValueHelperCall(keyText)) {
                        callable = nativeFunctionValue(keyText, describeValue(target));
                    } else if (isSquirrelValueMethod(keyText)) {
                        callable = valueMethodValue(keyText, describeValue(target));
                    } else if (isUiObjectMethod(keyText)) {
                        callable = uiMethodValue(keyText, describeValue(target));
                    }
                }
                if (!rootObjectReceiver.empty()) {
                    callable = valueMethodValue(keyText, rootObjectReceiver);
                }
                if (a2 > 0) {
                    std::string receiver;
                    if ((callable.kind == ScriptValueKind::Method || callable.kind == ScriptValueKind::UiMethod
                            || callable.kind == ScriptValueKind::ValueMethod)
                        && isConcreteReceiver(callable.receiver) && callable.receiver != "null") {
                        receiver = callable.receiver;
                    } else if (!rootObjectReceiver.empty()) {
                        receiver = rootObjectReceiver;
                    } else if ((target.kind == ScriptValueKind::Object || target.kind == ScriptValueKind::Class)
                        && isConcreteReceiver(target.text)) {
                        receiver = target.text;
                    }
                    if (!receiver.empty()) {
                        result.callReceivers[instruction.pc] = receiver;
                    }
                }
                setRegister(a3, target.kind == ScriptValueKind::Class ? getRegister(0) : target);
                setRegister(a0, callable);
            } else if (instruction.op == "_OP_CALL" || instruction.op == "_OP_TAILCALL") {
                const auto callArguments = getCallArguments(a2, a3);
                const auto callPc = callPcByCallableRegister.find(a1);
                if (callPc != callPcByCallableRegister.end()) {
                    result.callArguments[callPc->second] = callArguments;
                }
                bool syncHandled = false;
                if (callPc != callPcByCallableRegister.end()) {
                    const SynchronousCallResult syncResult =
                        executeScriptCallableForReturn(
                            getRegister(a1), callArguments, function, instruction, ownerObject, depth);
                    if (syncResult.handled) {
                        syncHandled = true;
                        result.synchronouslyExecutedCallPcs.insert(callPc->second);
                        if (isKnownValue(syncResult.value)) {
                            setRegister(a0, syncResult.value);
                        }
                    }
                }
                if (!syncHandled) {
                    const ScriptValue callResult =
                        makeCallReturn(function, instruction, getRegister(a1), callArguments);
                    if (isKnownValue(callResult)) {
                        setRegister(a0, callResult);
                    }
                }
            }
            ++i;
        }
        if (report_) {
            report_->bytecodeStateInstructions += result.executedInstructions;
        }
        return result;
    }

    void constructObject(
        const std::string& objectName,
        const std::string& className,
        int pc,
        int sourceLine,
        const std::string& evidence,
        int depth,
        const std::vector<ScriptValue>& constructorArguments = {})
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
            ensureRuntimeObject(objectName, className, "script_object");
            materializeClassDefaults(objectName, className);
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
            executeMethod(*constructor, objectName, className, depth + 1, "script_constructor", constructorArguments);
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

    ScriptValue executeObjectMethodSlot(
        const ScriptObjectMethodSlot& binding,
        const std::string& receiver,
        const SqasmCall& call,
        int depth,
        const std::vector<ScriptValue>& arguments = {})
    {
        const SqasmFunction* function = findFunctionByOrdinal(module_, binding.functionOrdinal);
        if (!function) {
            markUnresolved(call.name, receiver, {}, call.sourceLine, call.pc, "root object method ordinal not found");
            return {};
        }
        recordEvent({"execute_object_function", function->name, function->ordinal, receiver, {}, call.name, receiver,
            "script_object_function", {}, call.sourceLine, call.pc, binding.evidence});
        if (report_) {
            ++report_->scriptFunctions;
        }
        return executeFunction(*function, receiver, {}, depth + 1, arguments);
    }

    ScriptValue executeMethod(
        const ScriptMethodBinding& binding,
        const std::string& ownerObject,
        const std::string& ownerClass,
        int depth,
        const std::string& category,
        const std::vector<ScriptValue>& arguments = {})
    {
        if (depth > maxDepth_) {
            if (report_) {
                report_->truncated = true;
            }
            return {};
        }

        const SqasmModule& bindingModule = moduleForBinding(binding);
        const SqasmFunction* function = findFunctionByOrdinal(bindingModule, binding.functionOrdinal);
        if (!function) {
            markUnresolved(binding.slot, ownerObject, ownerClass, binding.sourceLine, binding.classPc,
                "method binding ordinal not found");
            return {};
        }

        const std::string activeKey =
            ownerObject + "|" + ownerClass + "|" + binding.slot + "|" + binding.modulePath + "|"
            + std::to_string(function->ordinal);
        if (activeMethods_.find(activeKey) != activeMethods_.end()) {
            recordEvent({"skip_recursive_method", function->name, function->ordinal, ownerObject, ownerClass,
                binding.slot, ownerObject, "recursion_guard", {}, binding.sourceLine, binding.classPc,
                "already active in call stack"});
            return {};
        }

        activeMethods_.insert(activeKey);
        recordEvent({"execute_method", function->name, function->ordinal, ownerObject, ownerClass, binding.slot,
            ownerObject, category, {}, binding.sourceLine, binding.classPc,
            "_OP_CLASS/_OP_CLOSURE/_OP_NEWSLOTA method binding"});
        if (report_) {
            ++report_->scriptMethods;
        }
        const ScriptValue result = executeFunction(*function, ownerObject, ownerClass, depth + 1, arguments);
        activeMethods_.erase(activeKey);
        return result;
    }

    ScriptValue executeFunction(
        const SqasmFunction& function,
        const std::string& ownerObject,
        const std::string& ownerClass,
        int depth,
        const std::vector<ScriptValue>& arguments = {})
    {
        if (depth > maxDepth_) {
            if (report_) {
                report_->truncated = true;
            }
            return {};
        }

        const BytecodeStateResult bytecodeState =
            executeBytecodeState(function, ownerObject, ownerClass, depth, arguments);
        for (const auto& call : function.calls) {
            if (bytecodeState.executedCallPcs.find(call.pc) == bytecodeState.executedCallPcs.end()) {
                continue;
            }
            if (bytecodeState.synchronouslyExecutedCallPcs.find(call.pc)
                != bytecodeState.synchronouslyExecutedCallPcs.end()) {
                continue;
            }
            std::string runtimeReceiver;
            const auto receiver = bytecodeState.callReceivers.find(call.pc);
            if (receiver != bytecodeState.callReceivers.end()) {
                runtimeReceiver = receiver->second;
            }
            std::vector<ScriptValue> runtimeArguments;
            const auto argumentIt = bytecodeState.callArguments.find(call.pc);
            if (argumentIt != bytecodeState.callArguments.end()) {
                runtimeArguments = argumentIt->second;
            }
            resolveAndExecuteCall(
                function, ownerObject, ownerClass, call, runtimeReceiver, runtimeArguments, depth + 1);
        }
        return bytecodeState.returnValue;
    }

    void resolveAndExecuteCall(
        const SqasmFunction& function,
        const std::string& ownerObject,
        const std::string& ownerClass,
        const SqasmCall& call,
        const std::string& runtimeReceiver,
        const std::vector<ScriptValue>& arguments,
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

        const std::string receiver = runtimeReceiver.empty() ? receiverForCall(function, call) : runtimeReceiver;
        if (!receiver.empty()) {
            if (executeReceiverCall(receiver, function, ownerObject, ownerClass, call, arguments, depth)) {
                return;
            }
        }

        if (executeSceneForeachCall(function, ownerObject, ownerClass, call, depth)) {
            return;
        }

        if (executeOwnerClassCall(ownerObject, ownerClass, call, arguments, depth)) {
            return;
        }

        if (executeClassConstructorCall(function, ownerObject, call, arguments, depth)) {
            return;
        }

        if (isModuleLifecycleCall(ownerClass, call.name)) {
            if (report_) {
                ++report_->moduleLifecycleCalls;
            }
            recordEvent({"module_lifecycle_call", function.name, function.ordinal, ownerObject, ownerClass, call.name,
                {}, "module_lifecycle_hook", {}, call.sourceLine, call.pc,
                "Module lifecycle hook recovered from boot-edge call trace; bytecode state captures boot edge, full ModuleBase semantics pending"});
            return;
        }

        if (dispatchNative(function, ownerObject, ownerClass, call)) {
            return;
        }

        if (isValueHelperCall(call.name)) {
            if (report_) {
                ++report_->valueHelperCalls;
            }
            recordEvent({"value_helper_call", function.name, function.ordinal, ownerObject, ownerClass, call.name,
                receiver, "script_value_helper", {}, call.sourceLine, call.pc,
                "value constructor/helper; ScriptValue return is typed by bytecode state interpreter"});
            return;
        }

        if (isSquirrelValueMethod(call.name)) {
            if (report_) {
                ++report_->valueMethodCalls;
            }
            recordEvent({"value_method_call", function.name, function.ordinal, ownerObject, ownerClass, call.name,
                receiver, "squirrel_value_method", {}, call.sourceLine, call.pc,
                "method on script/native return value; current return shape is typed by bytecode state interpreter"});
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
        const std::vector<ScriptValue>& arguments,
        int depth)
    {
        const ScriptObjectMethodSlot* objectSlot = findObjectMethodSlot(objectMethodSlots_, receiver, call.name);
        if (objectSlot) {
            executeObjectMethodSlot(*objectSlot, receiver, call, depth, arguments);
            return true;
        }

        const auto objectClass = objectClasses_.find(receiver);
        if (objectClass != objectClasses_.end()) {
            if (!isReturnSynchronizedScriptClass(objectClass->second)
                && isRuntimeOwnedScriptMethod(call.name)) {
                if (report_) {
                    ++report_->uiObjectCalls;
                }
                recordEvent({"runtime_owned_object_call", function.name, function.ordinal, ownerObject, ownerClass,
                    call.name, receiver, "runtime_owned_ui_helper_method", {}, call.sourceLine, call.pc,
                    "runtime service state already modeled this helper call during bytecode execution"});
                return true;
            }
            const ScriptMethodBinding* method = findMethodBindingInHierarchy(objectClass->second, call.name);
            if (method) {
                const std::string category =
                    method->ownerClass == objectClass->second ? "script_object_method" : "script_inherited_object_method";
                executeMethod(*method, receiver, method->ownerClass, depth, category, arguments);
                return true;
            }
        }

        if (hasClass(methodBindings_, receiver)) {
            const ScriptMethodBinding* method = findMethodBinding(methodBindings_, receiver, call.name);
            if (method) {
                const std::string targetObject = ownerObject.empty() ? receiver : ownerObject;
                executeMethod(*method, targetObject, receiver, depth, "script_super_or_class_method", arguments);
                return true;
            }
        }

        if (isScriptFieldReceiver(receiver) && isUiObjectMethod(call.name)) {
            if (report_) {
                ++report_->uiObjectCalls;
            }
            recordServiceState("UI And 2D Render Service", call.name, "ui_field_method_call", receiver,
                "arguments_pending", function, call,
                "call trace resolved a script field receiver as UI helper object; argument payload decoding pending");
            recordEvent({"ui_object_call", function.name, function.ordinal, ownerObject, ownerClass, call.name,
                receiver, "script_ui_helper_object", {}, call.sourceLine, call.pc,
                "field receiver UI helper; bytecode state interpreter tracks field mutation and return shape"});
            return true;
        }

        if (receiver.rfind("actor:", 0) == 0 && isActorRuntimeMethod(call.name)) {
            if (report_) {
                ++report_->engineObjectCalls;
            }
            recordEvent({"engine_object_call", function.name, function.ordinal, ownerObject, ownerClass, call.name,
                receiver, "runtime_owned_actor_method", "Actor And Task Service", call.sourceLine, call.pc,
                "actor method service state was modeled during bytecode execution"});
            return true;
        }

        if (receiver.rfind("loader:", 0) == 0 && isLoaderRuntimeMethod(call.name)) {
            if (report_) {
                ++report_->engineObjectCalls;
            }
            recordEvent({"engine_object_call", function.name, function.ordinal, ownerObject, ownerClass, call.name,
                receiver, "runtime_owned_loader_method", "Scene And Stage Service", call.sourceLine, call.pc,
                "loader method service state was modeled during bytecode execution"});
            return true;
        }

        if (receiver.rfind("place_params:", 0) == 0 && isPlaceParamsRuntimeMethod(call.name)) {
            if (report_) {
                ++report_->engineObjectCalls;
            }
            recordEvent({"engine_object_call", function.name, function.ordinal, ownerObject, ownerClass, call.name,
                receiver, "runtime_owned_place_params_method", "Scene And Stage Service", call.sourceLine, call.pc,
                "place parameter method service state was modeled during bytecode execution"});
            return true;
        }

        if (receiver == "ms" && isMissionRuntimeMethod(call.name)) {
            if (report_) {
                ++report_->engineObjectCalls;
            }
            recordEvent({"engine_object_call", function.name, function.ordinal, ownerObject, ownerClass, call.name,
                receiver, "runtime_owned_mission_method", "Event/Quest/Flag Service", call.sourceLine, call.pc,
                "mission runtime method service state was modeled during bytecode execution"});
            return true;
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
        const std::vector<ScriptValue>& arguments,
        int depth)
    {
        if (ownerClass.empty()) {
            return false;
        }

        std::string dispatchClass = ownerClass;
        const auto objectClass = objectClasses_.find(ownerObject);
        if (objectClass != objectClasses_.end() && !objectClass->second.empty()) {
            dispatchClass = objectClass->second;
        }

        const ScriptMethodBinding* method = findMethodBindingInHierarchy(dispatchClass, call.name);
        if (!method) {
            return false;
        }
        const std::string category = method->ownerClass == ownerClass
            ? "script_owner_method"
            : (dispatchClass == ownerClass ? "script_inherited_owner_method" : "script_virtual_owner_method");
        executeMethod(*method, ownerObject, method->ownerClass, depth, category, arguments);
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
            const ScriptMethodBinding* method = findMethodBindingInHierarchy(classIt->second, "init");
            if (method) {
                const std::string category =
                    method->ownerClass == classIt->second ? "script_scene_foreach_method"
                                                          : "script_inherited_scene_foreach_method";
                executeMethod(*method, sceneObject, method->ownerClass, depth, category);
            }
        }
        return true;
    }

    bool executeClassConstructorCall(
        const SqasmFunction& function,
        const std::string& ownerObject,
        const SqasmCall& call,
        const std::vector<ScriptValue>& arguments,
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
            depth,
            arguments);
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
    const std::vector<SqasmModule>& baselineModules_;
    const native::NativeRegistry& registry_;
    const native::NativeServiceCatalog& catalog_;
    ScriptRunOptions options_;
    std::vector<ScriptMethodBinding> methodBindings_;
    std::vector<ScriptObjectBinding> objectBindings_;
    std::vector<ScriptObjectMethodSlot> objectMethodSlots_;
    std::vector<FunctionSlotBinding> functionSlots_;
    std::map<std::string, std::string> objectClasses_;
    std::map<std::string, std::vector<std::string>> sceneObjectsByOwner_;
    std::map<std::string, RuntimeObject> runtimeObjects_;
    std::map<std::string, ScriptValue> rootFields_;
    std::map<std::string, std::map<std::string, ScriptValue>> classSlots_;
    std::map<std::string, std::string> classBases_;
    std::map<int, RuntimeTable> runtimeTables_;
    std::set<std::string> rootObjects_;
    std::set<std::string> activeMethods_;
    int nextTableId_ = 1;
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
    ScriptRunOptions options;
    options.frames = frames;
    return runEntryScript(module, entryFunction, registry, catalog, options);
}

ScriptExecutionReport runEntryScript(
    const SqasmModule& module,
    const std::string& entryFunction,
    const native::NativeRegistry& registry,
    const native::NativeServiceCatalog& catalog,
    const ScriptRunOptions& options)
{
    const std::vector<SqasmModule> baselineModules;
    StaticScriptExecutor executor(module, baselineModules, registry, catalog, options);
    return executor.run(entryFunction);
}

ScriptExecutionReport runEntryScript(
    const SqasmModule& module,
    const std::vector<SqasmModule>& baselineModules,
    const std::string& entryFunction,
    const native::NativeRegistry& registry,
    const native::NativeServiceCatalog& catalog,
    int frames)
{
    ScriptRunOptions options;
    options.frames = frames;
    return runEntryScript(module, baselineModules, entryFunction, registry, catalog, options);
}

ScriptExecutionReport runEntryScript(
    const SqasmModule& module,
    const std::vector<SqasmModule>& baselineModules,
    const std::string& entryFunction,
    const native::NativeRegistry& registry,
    const native::NativeServiceCatalog& catalog,
    const ScriptRunOptions& options)
{
    StaticScriptExecutor executor(module, baselineModules, registry, catalog, options);
    return executor.run(entryFunction);
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
        << " baseline_modules=" << report.baselineModules
        << " constructed_objects=" << report.constructedObjects << " script_methods=" << report.scriptMethods
        << " script_functions=" << report.scriptFunctions << " builtin_calls=" << report.builtinCalls
        << " native_obligations=" << report.nativeObligations
        << " native_implemented_calls=" << report.nativeImplementedCalls
        << " unique_native_apis=" << uniqueNativeApis.size() << " engine_object_calls="
        << report.engineObjectCalls << " ui_object_calls=" << report.uiObjectCalls
        << " value_helper_calls=" << report.valueHelperCalls << " value_method_calls=" << report.valueMethodCalls
        << " module_lifecycle_calls=" << report.moduleLifecycleCalls
        << " bytecode_state_functions=" << report.bytecodeStateFunctions
        << " bytecode_state_instructions=" << report.bytecodeStateInstructions
        << " root_slot_writes=" << report.rootSlotWrites << " class_slot_writes=" << report.classSlotWrites
        << " object_field_writes=" << report.objectFieldWrites << " table_slot_writes=" << report.tableSlotWrites
        << " typed_call_returns=" << report.typedCallReturns << " ui_object_mutations="
        << report.uiObjectMutations
        << " service_state_events=" << report.serviceStateEventCount
        << " save_service_queries=" << report.saveServiceQueries
        << " platform_state_queries=" << report.platformStateQueries
        << " audio_service_commands=" << report.audioServiceCommands
        << " scene_service_commands=" << report.sceneServiceCommands
        << " ui_objects_tracked=" << report.uiObjectsTracked
        << " ui_service_commands=" << report.uiServiceCommands
        << " value_state_queries=" << report.valueStateQueries
        << " decoded_service_arguments=" << report.decodedServiceArguments
        << " optional_unbound_globals=" << report.optionalUnboundGlobals
        << " unresolved_calls=" << report.unresolvedCalls << " truncated="
        << (report.truncated ? "true" : "false") << " status=" << core::jsonEscape(report.status) << "\",\n";
    out << "  \"entry_found\": " << (report.entryFound ? "true" : "false") << ",\n";
    out << "  \"executed\": " << (report.executed ? "true" : "false") << ",\n";
    out << "  \"frames\": " << report.frames << ",\n";
    out << "  \"status\": \"" << core::jsonEscape(report.status) << "\",\n";
    out << "  \"execution_mode\": \"" << core::jsonEscape(report.executionMode) << "\",\n";
    out << "  \"baseline_modules\": " << report.baselineModules << ",\n";
    out << "  \"baseline_module_paths\": ";
    writeStringArray(out, report.baselineModulePaths);
    out << ",\n";
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
    out << "  \"ui_object_calls\": " << report.uiObjectCalls << ",\n";
    out << "  \"value_helper_calls\": " << report.valueHelperCalls << ",\n";
    out << "  \"value_method_calls\": " << report.valueMethodCalls << ",\n";
    out << "  \"module_lifecycle_calls\": " << report.moduleLifecycleCalls << ",\n";
    out << "  \"bytecode_state_functions\": " << report.bytecodeStateFunctions << ",\n";
    out << "  \"bytecode_state_instructions\": " << report.bytecodeStateInstructions << ",\n";
    out << "  \"root_slot_writes\": " << report.rootSlotWrites << ",\n";
    out << "  \"class_slot_writes\": " << report.classSlotWrites << ",\n";
    out << "  \"object_field_writes\": " << report.objectFieldWrites << ",\n";
    out << "  \"table_slot_writes\": " << report.tableSlotWrites << ",\n";
    out << "  \"typed_call_returns\": " << report.typedCallReturns << ",\n";
    out << "  \"ui_object_mutations\": " << report.uiObjectMutations << ",\n";
    out << "  \"service_state_events\": " << report.serviceStateEventCount << ",\n";
    out << "  \"save_service_queries\": " << report.saveServiceQueries << ",\n";
    out << "  \"platform_state_queries\": " << report.platformStateQueries << ",\n";
    out << "  \"audio_service_commands\": " << report.audioServiceCommands << ",\n";
    out << "  \"scene_service_commands\": " << report.sceneServiceCommands << ",\n";
    out << "  \"ui_objects_tracked\": " << report.uiObjectsTracked << ",\n";
    out << "  \"ui_service_commands\": " << report.uiServiceCommands << ",\n";
    out << "  \"value_state_queries\": " << report.valueStateQueries << ",\n";
    out << "  \"decoded_service_arguments\": " << report.decodedServiceArguments << ",\n";
    out << "  \"optional_unbound_globals\": " << report.optionalUnboundGlobals << ",\n";
    out << "  \"unresolved_calls\": " << report.unresolvedCalls << ",\n";
    out << "  \"truncated\": " << (report.truncated ? "true" : "false") << ",\n";
    out << "  \"unique_native_api_names\": ";
    const std::vector<std::string> uniqueNativeApiNames(uniqueNativeApis.begin(), uniqueNativeApis.end());
    writeStringArray(out, uniqueNativeApiNames);
    out << ",\n";
    out << "  \"runtime_input_state\": "
        << (report.runtimeInputStateJson.empty() ? "{}" : report.runtimeInputStateJson) << ",\n";
    out << "  \"runtime_service_state\": "
        << (report.runtimeServiceStateJson.empty() ? "{}" : report.runtimeServiceStateJson) << ",\n";
    out << "  \"runtime_script_state\": "
        << (report.runtimeScriptStateJson.empty() ? "{}" : report.runtimeScriptStateJson) << ",\n";
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
    out << "  \"service_state_event_details\": [\n";
    for (size_t i = 0; i < report.serviceStateEvents.size(); ++i) {
        const auto& event = report.serviceStateEvents[i];
        out << "    {\"service\": \"" << core::jsonEscape(event.service) << "\", \"api\": \""
            << core::jsonEscape(event.api) << "\", \"action\": \"" << core::jsonEscape(event.action)
            << "\", \"target\": \"" << core::jsonEscape(event.target) << "\", \"value\": \""
            << core::jsonEscape(event.value) << "\", \"arguments\": \""
            << core::jsonEscape(event.arguments) << "\", \"function\": \""
            << core::jsonEscape(event.functionName) << "\", \"function_ordinal\": "
            << event.functionOrdinal << ", \"source_line\": " << event.sourceLine << ", \"pc\": " << event.pc
            << ", \"evidence\": \""
            << core::jsonEscape(event.evidence) << "\"}";
        out << (i + 1 == report.serviceStateEvents.size() ? "\n" : ",\n");
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
