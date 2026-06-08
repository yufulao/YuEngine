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

struct ScriptExecutionReport {
    std::string modulePath;
    std::string entryFunction;
    bool entryFound = false;
    bool executed = false;
    int frames = 0;
    std::string status;
    std::string executionMode;
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
    int optionalUnboundGlobals = 0;
    int unresolvedCalls = 0;
    bool truncated = false;
    std::vector<ScriptConstructedObject> constructedObjectDetails;
    std::vector<ScriptObjectMethodSlot> objectMethodSlots;
    std::vector<ScriptNativeObligation> obligations;
    std::vector<ScriptExecutionEvent> events;
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

std::string scriptExecutionReportToJson(const ScriptExecutionReport& report);

} // namespace yu::script
