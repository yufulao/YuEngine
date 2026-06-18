// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldObjectIdentityStatus.h

#pragma once

namespace yuengine::world {
enum class WorldObjectIdentityStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidWorldObjectId,
    MissingWorldObject,
    InvalidObjectHandle,
    StaleObjectHandle,
    DuplicateWorldObjectId,
    DuplicateObjectHandle,
    CapacityExceeded,
    BindingNotFound,
    ObjectAcquireFailed,
    ObjectReleaseFailed
};
}
