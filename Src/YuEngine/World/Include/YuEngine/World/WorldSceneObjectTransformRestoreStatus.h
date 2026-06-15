// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformRestoreStatus.h

#pragma once

namespace yuengine::world {
enum class WorldSceneObjectTransformRestoreStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidWorld,
    InvalidObjectRegistry,
    InvalidIdentityDestination,
    InvalidTransformDestination,
    InvalidIdentityInput,
    InvalidTransformInput,
    InputCountExceeded,
    IdentityCapacityExceeded,
    TransformCapacityExceeded,
    DestinationNotEmpty,
    InvalidWorldObjectId,
    MissingWorldObject,
    InvalidObjectHandle,
    StaleObjectHandle,
    ObjectAcquireWouldOverflow,
    DuplicateIdentityWorldObjectId,
    DuplicateIdentityObjectHandle,
    DuplicateTransformWorldObjectId,
    MissingIdentityForTransform,
    IdentityApplyFailed,
    TransformApplyFailed,
    ObjectAcquireFailed
};
}
