// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyStatus.h

#pragma once

namespace yuengine::world {
enum class WorldSceneAssemblyStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidAttachmentDestination,
    InvalidBindingDestination,
    InvalidResourceRegistry,
    InvalidAttachmentInput,
    InvalidBindingInput,
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
    DuplicateAttachmentInput,
    DuplicateBindingInput,
    AttachmentCapacityExceeded,
    BindingCapacityExceeded,
    DestinationNotEmpty,
    AttachmentApplyFailed,
    BindingRestoreFailed,
    ResourceAcquireFailed,
    RollbackFailed
};
}
