// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSerializeSnapshotStatus.h

#pragma once

namespace yuengine::world {
enum class WorldSerializeSnapshotStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidWriter,
    InvalidReader,
    InvalidWorldSnapshot,
    InvalidTransformSnapshot,
    InvalidTraceBuffer,
    TraceCapacityExceeded,
    InvalidOutputWorldSnapshot,
    InvalidOutputTraceCount,
    InvalidOutputTraceBuffer,
    InvalidEnumValue,
    SerializeFailure
};
}
