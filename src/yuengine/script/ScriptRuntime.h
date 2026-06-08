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
    std::string category;
    std::string service;
    std::string implementationStatus;
    std::vector<int> candidateOrdinals;
};

struct ScriptExecutionPlan {
    std::string modulePath;
    std::string entryFunction;
    bool entryFound = false;
    int entryOrdinal = -1;
    int entryInstructions = 0;
    std::string status;
    int directCalls = 0;
    int nativeObligations = 0;
    int scriptCalls = 0;
    int ambiguousCalls = 0;
    int unresolvedCalls = 0;
    std::vector<ScriptCallResolution> callResolutions;
};

ScriptExecutionPlan planEntryExecution(
    const SqasmModule& module,
    const std::string& entryFunction,
    const native::NativeRegistry& registry);

std::string scriptExecutionPlanToJson(const ScriptExecutionPlan& plan);

} // namespace yu::script
