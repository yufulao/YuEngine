// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingRestoreStatus.h

#pragma once

namespace yuengine::world {
enum class WorldComponentResourceBindingRestoreStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidDestinationBridge,
    InvalidAttachmentSource,
    InvalidResourceRegistry,
    InvalidInput,
    InputCountExceeded,
    InvalidWorldObjectId,
    InvalidComponentTypeId,
    InvalidComponentSlotId,
    MissingAttachment,
    InvalidResourceHandle,
    StaleResourceHandle,
    InvalidResourceTypeId,
    ResourceTypeMismatch,
    ResourceAcquireWouldOverflow,
    DuplicateInputBinding,
    DestinationCapacityExceeded,
    DestinationNotEmpty,
    ResourceAcquireFailed,
    BindingApplyFailed,
    RollbackFailed
};
}
