// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldStatus.h

#pragma once

namespace yuengine::world {
enum class WorldStatus {
    Success,
    InvalidObjectCapacity,
    InvalidPhaseTraceCapacity,
    InvalidObjectId,
    DuplicateObjectId,
    ObjectNotFound,
    CapacityExceeded,
    InvalidLifecycleState,
    InvalidTimeStep,
    InvalidTraceBuffer
};
}
