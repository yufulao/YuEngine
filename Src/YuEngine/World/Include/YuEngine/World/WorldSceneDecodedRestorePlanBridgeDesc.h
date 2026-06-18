// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneDecodedRestorePlanBridgeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanConstants.h"

namespace yuengine::world {
struct WorldSceneDecodedRestorePlanBridgeDesc final {
    std::uint32_t identity_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t transform_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t attachment_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t binding_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t plan_capacity = MAX_WORLD_SCENE_DECODED_RESTORE_PLAN_RECORD_COUNT;
};
}
