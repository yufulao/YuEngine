// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneDecodedRestorePlanStatus.h

#pragma once

namespace yuengine::world {
enum class WorldSceneDecodedRestorePlanStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidWorld,
    InvalidObjectRegistry,
    InvalidResourceRegistry,
    InvalidIdentityDestination,
    InvalidTransformDestination,
    InvalidAttachmentDestination,
    InvalidBindingDestination,
    InvalidIdentityInput,
    InvalidTransformInput,
    InvalidAttachmentInput,
    InvalidBindingInput,
    InvalidPlanOutput,
    PlanOutputCapacityExceeded,
    IdentityCapacityExceeded,
    TransformCapacityExceeded,
    AttachmentCapacityExceeded,
    BindingCapacityExceeded,
    DestinationNotEmpty,
    InvalidWorldObjectId,
    MissingWorldObject,
    InvalidObjectHandle,
    StaleObjectHandle,
    ObjectAcquireWouldOverflow,
    InvalidComponentTypeId,
    InvalidComponentSlotId,
    MissingIdentityForTransform,
    MissingIdentityForAttachment,
    MissingIdentityForBinding,
    MissingAttachmentForBinding,
    InvalidResourceHandle,
    StaleResourceHandle,
    InvalidResourceTypeId,
    ResourceTypeMismatch,
    ResourceAcquireWouldOverflow,
    DuplicateIdentityWorldObjectId,
    DuplicateIdentityObjectHandle,
    DuplicateTransformWorldObjectId,
    DuplicateAttachmentTuple,
    DuplicateBindingTuple,
    ObjectAcquireFailed,
    ResourceAcquireFailed
};
}
