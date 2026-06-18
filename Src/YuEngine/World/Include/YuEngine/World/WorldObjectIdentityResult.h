// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldObjectIdentityResult.h

#pragma once

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"

namespace yuengine::world {
struct WorldObjectIdentityResult final {
    WorldObjectIdentityStatus status = WorldObjectIdentityStatus::Success;
    WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};

    /**
     * @comment 创建成功 result。
     * @param world_object_id 输入 world object id。
     * @param object_handle 输入 object handle。
     * @return 显式操作结果。
     */
    static WorldObjectIdentityResult Success(WorldObjectId world_object_id,
        yuengine::object::ObjectHandle object_handle) {
        return WorldObjectIdentityResult{
            WorldObjectIdentityStatus::Success,
            world_object_id,
            object_handle};
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 bridge status。
     * @return 显式操作结果。
     */
    static WorldObjectIdentityResult Failure(WorldObjectIdentityStatus status) {
        return WorldObjectIdentityResult{status, WorldObjectId{}, yuengine::object::ObjectHandle{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldObjectIdentityStatus::Success;
    }
};
}
