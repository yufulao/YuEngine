// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformRestoreResult.h

#pragma once

#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreState.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreStatus.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
struct WorldSceneObjectTransformRestoreResult final {
    WorldSceneObjectTransformRestoreStatus status =
        WorldSceneObjectTransformRestoreStatus::Success;
    WorldObjectIdentityStatus identity_status = WorldObjectIdentityStatus::Success;
    WorldTransformStatus transform_status = WorldTransformStatus::Success;
    yuengine::object::ObjectStatus object_status = yuengine::object::ObjectStatus::Success;
    WorldSceneObjectTransformRestoreState state{};

    /**
     * @comment 创建成功 object-transform restore result。
     * @param state 输入 operation state。
     * @return 显式操作结果。
     */
    static WorldSceneObjectTransformRestoreResult Success(
        const WorldSceneObjectTransformRestoreState &state) {
        return WorldSceneObjectTransformRestoreResult{
            WorldSceneObjectTransformRestoreStatus::Success,
            WorldObjectIdentityStatus::Success,
            WorldTransformStatus::Success,
            yuengine::object::ObjectStatus::Success,
            state};
    }

    /**
     * @comment 创建失败 object-transform restore result。
     * @param status 输入 restore status。
     * @param identity_status 输入 identity bridge status。
     * @param transform_status 输入 transform bridge status。
     * @param object_status 输入 object registry status。
     * @return 显式操作结果。
     */
    static WorldSceneObjectTransformRestoreResult Failure(
        WorldSceneObjectTransformRestoreStatus status,
        WorldObjectIdentityStatus identity_status=WorldObjectIdentityStatus::Success,
        WorldTransformStatus transform_status=WorldTransformStatus::Success,
        yuengine::object::ObjectStatus object_status=
            yuengine::object::ObjectStatus::Success) {
        return WorldSceneObjectTransformRestoreResult{
            status,
            identity_status,
            transform_status,
            object_status,
            WorldSceneObjectTransformRestoreState{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldSceneObjectTransformRestoreStatus::Success;
    }
};
}
