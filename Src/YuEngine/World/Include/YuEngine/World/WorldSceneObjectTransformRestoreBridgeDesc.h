// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformRestoreBridgeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"

namespace yuengine::world {
struct WorldSceneObjectTransformRestoreBridgeDesc final {
    std::uint32_t identity_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t transform_capacity = MAX_WORLD_OBJECT_COUNT;
};
}
