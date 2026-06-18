// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldTransformStatus.h

#pragma once

namespace yuengine::world {
enum class WorldTransformStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidWorldObjectId,
    MissingWorldObject,
    DuplicateWorldObjectId,
    CapacityExceeded,
    TransformNotFound
};
}
