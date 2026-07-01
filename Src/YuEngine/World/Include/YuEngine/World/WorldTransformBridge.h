// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldTransformBridge.h

#pragma once

#include <array>

#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldTransformBinding.h"
#include "YuEngine/World/WorldTransformBridgeDesc.h"
#include "YuEngine/World/WorldTransformResult.h"
#include "YuEngine/World/WorldTransformSnapshot.h"
#include "YuEngine/World/WorldTransformState.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
class WorldTransformBridge final {
public:
    /**
     * @comment 构造 transform data bridge for one WorldInstance。
     * @param world bridge 查询的 World instance。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldTransformBridge(WorldInstance &world,
        WorldTransformBridgeDesc desc=WorldTransformBridgeDesc{});

    /**
     * @comment 为 existing world object 注册 transform state。
     * @param world_object_id 输入 world object id。
     * @param transform_state 输入 transform state。
     * @return 显式操作结果。
     */
    WorldTransformResult Register(WorldObjectId world_object_id,
        const WorldTransformState &transform_state);
    /**
     * @comment 更新 existing transform record 的 transform state。
     * @param world_object_id 输入 world object id。
     * @param transform_state 输入 transform state。
     * @return 显式操作状态。
     */
    WorldTransformStatus Set(WorldObjectId world_object_id,
        const WorldTransformState &transform_state);
    /**
     * @comment 查询 existing transform record 的 transform state。
     * @param world_object_id 输入 world object id。
     * @return 显式操作结果。
     */
    WorldTransformResult Query(WorldObjectId world_object_id);
    /**
     * @comment 移除一个 transform record。
     * @param world_object_id 输入 world object id。
     * @return 显式操作状态。
     */
    WorldTransformStatus Remove(WorldObjectId world_object_id);
    /**
     * @comment 移除所有 transform records。
     * @return 显式操作状态。
     */
    WorldTransformStatus Clear();
    /**
     * @comment 返回当前 transform bridge 状态快照。
     * @return 快照值。
     */
    WorldTransformSnapshot Snapshot() const;

private:
    WorldTransformStatus RecordCapacityFailure(WorldObjectId world_object_id,
        const WorldTransformState &transform_state,
        std::uint32_t transform_slot);
    WorldTransformStatus RecordFailure(WorldTransformStatus status);
    void RecordSuccess();
    void ClearCapacityEntry();
    WorldTransformStatus ValidateBridgeCapacity() const;
    WorldTransformBinding *FindBindingByWorldObjectId(WorldObjectId world_object_id);
    const WorldTransformBinding *FindBindingByWorldObjectId(WorldObjectId world_object_id) const;
    WorldTransformBinding *FindFreeBinding();
    void ClearBinding(WorldTransformBinding &binding);
    void RecountRecords();

    WorldInstance &world_;
    std::array<WorldTransformBinding, MAX_WORLD_OBJECT_COUNT> bindings_;
    WorldTransformSnapshot snapshot_;
};
}
