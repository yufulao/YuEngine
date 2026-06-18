// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldResourceBindingBridge.h

#pragma once

#include <array>

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldResourceBinding.h"
#include "YuEngine/World/WorldResourceBindingBridgeDesc.h"
#include "YuEngine/World/WorldResourceBindingResult.h"
#include "YuEngine/World/WorldResourceBindingSnapshot.h"
#include "YuEngine/World/WorldResourceBindingStatus.h"

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::world {
class WorldResourceBindingBridge final {
public:
    /**
     * @comment 构造 world resource binding bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldResourceBindingBridge(
        WorldResourceBindingBridgeDesc desc=WorldResourceBindingBridgeDesc{});

    /**
     * @comment 将一个调用方提供的 world object id 绑定到一个 resource handle，并 acquire 该 handle。
     * @param resource_registry 调用方持有的 resource registry。
     * @param world_object_id 输入 world object id。
     * @param resource_handle 输入 resource handle。
     * @param expected_resource_type 输入 expected resource type。
     * @return 显式操作结果。
     */
    WorldResourceBindingResult Bind(
        yuengine::resource::ResourceRegistry *resource_registry,
        WorldObjectId world_object_id,
        yuengine::resource::ResourceHandle resource_handle,
        yuengine::resource::ResourceTypeId expected_resource_type);
    /**
     * @comment 按 world object id 查询一个 binding。
     * @param world_object_id 输入 world object id。
     * @return 显式操作结果。
     */
    WorldResourceBindingResult Query(WorldObjectId world_object_id);
    /**
     * @comment 移除一个 binding，并释放已 acquire 的 resource handle。
     * @param resource_registry 调用方持有的 resource registry。
     * @param world_object_id 输入 world object id。
     * @return 显式操作状态。
     */
    WorldResourceBindingStatus Remove(
        yuengine::resource::ResourceRegistry *resource_registry,
        WorldObjectId world_object_id);
    /**
     * @comment 释放并清空 deterministic slot order 中的所有 active bindings。
     * @param resource_registry 调用方持有的 resource registry。
     * @return 显式操作状态。
     */
    WorldResourceBindingStatus Clear(yuengine::resource::ResourceRegistry *resource_registry);
    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
     */
    WorldResourceBindingSnapshot Snapshot() const;

private:
    WorldResourceBindingResult RecordFailureResult(WorldResourceBindingStatus status);
    WorldResourceBindingResult RecordFailureResult(WorldResourceBindingStatus status,
        yuengine::resource::ResourceStatus resource_status);
    WorldResourceBindingStatus RecordFailure(WorldResourceBindingStatus status);
    WorldResourceBindingStatus RecordFailure(WorldResourceBindingStatus status,
        yuengine::resource::ResourceStatus resource_status);
    void RecordSuccess(yuengine::resource::ResourceStatus resource_status);
    WorldResourceBindingStatus ValidateBridgeCapacity() const;
    WorldResourceBindingStatus MapAcquireStatus(yuengine::resource::ResourceStatus resource_status) const;
    WorldResourceBinding *FindBindingByWorldObjectId(WorldObjectId world_object_id);
    const WorldResourceBinding *FindBindingByWorldObjectId(WorldObjectId world_object_id) const;
    WorldResourceBinding *FindFreeBinding();
    void ClearBinding(WorldResourceBinding &binding);
    void RecountActiveBindings();

    std::array<WorldResourceBinding, MAX_WORLD_OBJECT_COUNT> bindings_;
    WorldResourceBindingSnapshot snapshot_;
};
}
