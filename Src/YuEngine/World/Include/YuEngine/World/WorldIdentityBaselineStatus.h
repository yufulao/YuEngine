// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldIdentityBaselineStatus.h

#pragma once

namespace yuengine::world {
enum class WorldIdentityBaselineStatus {
    Success,
    InvalidBaselineCapacity,
    CapacityExceeded,
    InvalidWorldObjectId,
    InvalidObjectType,
    InvalidObjectReferenceCount,
    InvalidComponentTypeId,
    InvalidComponentSlotId,
    DuplicateWorldObjectId,
    RecordNotFound,
    WorldObjectMissing,
    ObjectCreateFailed,
    ObjectValidateFailed,
    ObjectDestroyFailed,
    WorldRegisterFailed,
    WorldRemoveFailed,
    IdentityBindFailed,
    IdentityValidateFailed,
    IdentityRemoveFailed,
    TransformRegisterFailed,
    TransformValidateFailed,
    TransformRemoveFailed,
    ComponentAttachFailed,
    ComponentValidateFailed,
    ComponentRemoveFailed
};
}
