// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneApplyTimeRestoreProofBridgeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofConstants.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanConstants.h"

namespace yuengine::world {
struct WorldSceneApplyTimeRestoreProofBridgeDesc final {
    std::uint32_t identity_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t transform_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t attachment_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t binding_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t plan_scratch_capacity =
        MAX_WORLD_SCENE_DECODED_RESTORE_PLAN_RECORD_COUNT;
    std::uint32_t proof_capacity =
        MAX_WORLD_SCENE_APPLY_TIME_RESTORE_PROOF_RECORD_COUNT;
    std::uint32_t slice_capacity =
        MAX_WORLD_SCENE_APPLY_TIME_RESTORE_SLICE_RECORD_COUNT;
};
}
