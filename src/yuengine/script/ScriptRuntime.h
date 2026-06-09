#pragma once

#include "yuengine/native/NativeRegistry.h"
#include "yuengine/native/NativeServices.h"
#include "yuengine/script/SqasmModule.h"

#include <string>
#include <vector>

namespace yu::script {

struct ScriptCallResolution {
    std::string name;
    int pc = -1;
    int sourceLine = -1;
    std::string receiver;
    std::string ownerClass;
    std::string category;
    std::string service;
    std::string implementationStatus;
    std::string evidence;
    std::vector<int> candidateOrdinals;
};

struct ScriptMethodBinding {
    std::string modulePath;
    std::string ownerClass;
    int classPc = -1;
    int sourceLine = -1;
    std::string slot;
    int functionRefIndex = -1;
    int functionOrdinal = -1;
    std::string functionName;
};

struct ScriptObjectBinding {
    std::string objectName;
    std::string className;
    int pc = -1;
    int sourceLine = -1;
    std::string evidence;
};

struct ScriptObjectMethodSlot {
    std::string objectName;
    std::string slot;
    int functionRefIndex = -1;
    int functionOrdinal = -1;
    std::string functionName;
    int pc = -1;
    int sourceLine = -1;
    std::string evidence;
};

struct ScriptExecutionPlan {
    std::string modulePath;
    std::string entryFunction;
    bool entryFound = false;
    int entryOrdinal = -1;
    int entryInstructions = 0;
    std::string status;
    int directCalls = 0;
    int builtinCalls = 0;
    int nativeObligations = 0;
    int objectMethodCalls = 0;
    int scriptCalls = 0;
    int ambiguousCalls = 0;
    int unresolvedCalls = 0;
    int classMethodTableCount = 0;
    int methodBindingCount = 0;
    int objectBindingCount = 0;
    std::vector<ScriptMethodBinding> methodBindings;
    std::vector<ScriptObjectBinding> objectBindings;
    std::vector<ScriptCallResolution> callResolutions;
};

struct ScriptConstructedObject {
    std::string objectName;
    std::string className;
    int pc = -1;
    int sourceLine = -1;
    std::string evidence;
};

struct ScriptNativeObligation {
    std::string api;
    std::string service;
    std::string ownerLevel;
    std::string implementationStatus;
    std::string functionName;
    int functionOrdinal = -1;
    int sourceLine = -1;
    int pc = -1;
    std::string obligation;
};

struct ScriptExecutionEvent {
    std::string kind;
    std::string functionName;
    int functionOrdinal = -1;
    std::string ownerObject;
    std::string ownerClass;
    std::string callName;
    std::string receiver;
    std::string category;
    std::string service;
    int sourceLine = -1;
    int pc = -1;
    std::string evidence;
};

struct ScriptServiceStateEvent {
    std::string service;
    std::string api;
    std::string action;
    std::string target;
    std::string value;
    std::string arguments;
    std::string functionName;
    int functionOrdinal = -1;
    int sourceLine = -1;
    int pc = -1;
    std::string evidence;
};

struct ScriptExecutionReport {
    std::string modulePath;
    std::string entryFunction;
    bool entryFound = false;
    bool executed = false;
    int frames = 0;
    std::string status;
    std::string executionMode;
    int baselineModules = 0;
    int classMethodTableCount = 0;
    int methodBindingCount = 0;
    int objectBindingCount = 0;
    int rootObjectMethodSlots = 0;
    int constructedObjects = 0;
    int scriptMethods = 0;
    int scriptFunctions = 0;
    int builtinCalls = 0;
    int nativeObligations = 0;
    int nativeImplementedCalls = 0;
    int engineObjectCalls = 0;
    int uiObjectCalls = 0;
    int valueHelperCalls = 0;
    int valueMethodCalls = 0;
    int moduleLifecycleCalls = 0;
    int bytecodeStateFunctions = 0;
    int bytecodeStateInstructions = 0;
    int rootSlotWrites = 0;
    int classSlotWrites = 0;
    int objectFieldWrites = 0;
    int tableSlotWrites = 0;
    int typedCallReturns = 0;
    int uiObjectMutations = 0;
    int serviceStateEventCount = 0;
    int saveServiceQueries = 0;
    int platformStateQueries = 0;
    int audioServiceCommands = 0;
    int sceneServiceCommands = 0;
    int uiObjectsTracked = 0;
    int uiServiceCommands = 0;
    int valueStateQueries = 0;
    int decodedServiceArguments = 0;
    int optionalUnboundGlobals = 0;
    int unresolvedCalls = 0;
    bool truncated = false;
    std::string runtimeInputStateJson;
    std::string runtimeServiceStateJson;
    std::string runtimeScriptStateJson;
    std::vector<std::string> baselineModulePaths;
    std::vector<ScriptConstructedObject> constructedObjectDetails;
    std::vector<ScriptObjectMethodSlot> objectMethodSlots;
    std::vector<ScriptNativeObligation> obligations;
    std::vector<ScriptServiceStateEvent> serviceStateEvents;
    std::vector<ScriptExecutionEvent> events;
};

struct ScriptRunOptions {
    int frames = 0;
    int renderFrames = 0;
    std::string inputScenario = "passive";
    int menuSelectedIndex = 0;
    bool menuDecide = false;
    bool menuDown = false;
    bool menuUp = false;
    bool saveListEmpty = true;
    bool continueDisabled = true;
    bool canShutdown = false;
    bool executeEventSetupScripts = false;
};

ScriptExecutionPlan planEntryExecution(
    const SqasmModule& module,
    const std::string& entryFunction,
    const native::NativeRegistry& registry);

std::string scriptExecutionPlanToJson(const ScriptExecutionPlan& plan);

ScriptExecutionReport runEntryScript(
    const SqasmModule& module,
    const std::string& entryFunction,
    const native::NativeRegistry& registry,
    const native::NativeServiceCatalog& catalog,
    int frames);

ScriptExecutionReport runEntryScript(
    const SqasmModule& module,
    const std::string& entryFunction,
    const native::NativeRegistry& registry,
    const native::NativeServiceCatalog& catalog,
    const ScriptRunOptions& options);

ScriptExecutionReport runEntryScript(
    const SqasmModule& module,
    const std::vector<SqasmModule>& baselineModules,
    const std::string& entryFunction,
    const native::NativeRegistry& registry,
    const native::NativeServiceCatalog& catalog,
    int frames);

ScriptExecutionReport runEntryScript(
    const SqasmModule& module,
    const std::vector<SqasmModule>& baselineModules,
    const std::string& entryFunction,
    const native::NativeRegistry& registry,
    const native::NativeServiceCatalog& catalog,
    const ScriptRunOptions& options);

std::string scriptExecutionReportToJson(const ScriptExecutionReport& report);

} // namespace yu::script
