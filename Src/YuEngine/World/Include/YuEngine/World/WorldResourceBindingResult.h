// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldResourceBindingResult.h

#pragma once

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldResourceBindingStatus.h"

namespace yuengine::world {
struct WorldResourceBindingResult final {
    WorldResourceBindingStatus status = WorldResourceBindingStatus::Success;
    yuengine::resource::ResourceStatus resource_status = yuengine::resource::ResourceStatus::Success;
    WorldObjectId world_object_id{};
    yuengine::resource::ResourceHandle resource_handle{};
    yuengine::resource::ResourceTypeId expected_resource_type{};

    /**
     * @comment 创建成功 result。
     * @param world_object_id 输入 world object id。
     * @param resource_handle 输入 resource handle。
     * @param expected_resource_type 输入 expected resource type。
     * @return 显式操作结果。
     */
    static WorldResourceBindingResult Success(WorldObjectId world_object_id,
        yuengine::resource::ResourceHandle resource_handle,
        yuengine::resource::ResourceTypeId expected_resource_type) {
        return WorldResourceBindingResult{
            WorldResourceBindingStatus::Success,
            yuengine::resource::ResourceStatus::Success,
            world_object_id,
            resource_handle,
            expected_resource_type};
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 bridge status。
     * @param resource_status 输入 resource status。
     * @return 显式操作结果。
     */
    static WorldResourceBindingResult Failure(WorldResourceBindingStatus status,
        yuengine::resource::ResourceStatus resource_status=yuengine::resource::ResourceStatus::Success) {
        return WorldResourceBindingResult{
            status,
            resource_status,
            WorldObjectId{},
            yuengine::resource::ResourceHandle{},
            yuengine::resource::ResourceTypeId{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldResourceBindingStatus::Success;
    }
};
}
