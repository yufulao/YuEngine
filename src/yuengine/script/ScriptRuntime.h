#pragma once

#include "yuengine/native/NativeRegistry.h"
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

ScriptExecutionPlan planEntryExecution(
    const SqasmModule& module,
    const std::string& entryFunction,
    const native::NativeRegistry& registry);

std::string scriptExecutionPlanToJson(const ScriptExecutionPlan& plan);

} // namespace yu::script
