// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldTransformResult.h

#pragma once

#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
struct WorldTransformResult final {
    WorldTransformStatus status = WorldTransformStatus::Success;
    WorldObjectId world_object_id{};
    WorldTransformState transform_state{};

    /**
     * @comment 创建成功 result。
     * @param world_object_id 输入 world object id。
     * @param transform_state 输入 transform state。
     * @return 显式操作结果。
     */
    static WorldTransformResult Success(WorldObjectId world_object_id,
        const WorldTransformState &transform_state) {
        return WorldTransformResult{WorldTransformStatus::Success, world_object_id, transform_state};
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 transform status。
     * @return 显式操作结果。
     */
    static WorldTransformResult Failure(WorldTransformStatus status) {
        return WorldTransformResult{status, WorldObjectId{}, WorldTransformState{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldTransformStatus::Success;
    }
};
}
