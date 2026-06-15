// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldTransformStatus.h

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
