// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldResourceBindingStatus.h

#pragma once

namespace yuengine::world {
enum class WorldResourceBindingStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidResourceRegistry,
    InvalidWorldObjectId,
    InvalidResourceHandle,
    StaleResourceHandle,
    ResourceTypeMismatch,
    DuplicateWorldObjectId,
    CapacityExceeded,
    BindingNotFound,
    ResourceAcquireFailed,
    ResourceReleaseFailed
};
}
