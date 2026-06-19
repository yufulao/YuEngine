// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptRuntimePhaseDispatchStatus.h

#pragma once

namespace yuengine::script {
enum class ScriptRuntimePhaseDispatchStatus {
    Success,
    InvalidBindingCapacity,
    InvalidTraceCapacity,
    CapacityExceeded,
    InvalidPhase,
    InvalidCallId,
    DuplicatePhase,
    InvalidTraceBuffer,
    TraceCapacityExceeded,
    InvalidArgumentBuffer,
    InvalidResultBuffer,
    MissingCall,
    InvalidScriptSlot,
    ScriptCallFailed
};
}
