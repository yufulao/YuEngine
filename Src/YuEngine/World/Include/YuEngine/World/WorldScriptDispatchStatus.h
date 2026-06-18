// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldScriptDispatchStatus.h

#pragma once

namespace yuengine::world {
enum class WorldScriptDispatchStatus {
    Success,
    InvalidBindingCapacity,
    InvalidPhase,
    InvalidCallId,
    DuplicatePhase,
    CapacityExceeded,
    InvalidTraceBuffer,
    TraceCapacityExceeded,
    InvalidArgumentBuffer,
    InvalidResultBuffer,
    ScriptCallFailed
};
}
