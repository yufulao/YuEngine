// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingSnapshotStatus.h

#pragma once

namespace yuengine::world {
enum class WorldComponentResourceBindingSnapshotStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidSourceBridge,
    InvalidWriter,
    InvalidReader,
    InvalidOutput,
    InvalidOutputCount,
    UnsupportedVersion,
    MalformedRecordCount,
    RecordCountExceeded,
    OutputCapacityExceeded,
    InvalidWorldObjectId,
    InvalidComponentTypeId,
    InvalidComponentSlotId,
    InvalidResourceHandle,
    InvalidResourceTypeId,
    DuplicateBinding,
    SerializeFailure
};
}
