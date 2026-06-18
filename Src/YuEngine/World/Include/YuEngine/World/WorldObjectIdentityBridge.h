// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldObjectIdentityBridge.h

#pragma once

#include <array>

#include "YuEngine/Object/ObjectRegistry.h"
#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldObjectIdentityBinding.h"
#include "YuEngine/World/WorldObjectIdentityBridgeDesc.h"
#include "YuEngine/World/WorldObjectIdentityResult.h"
#include "YuEngine/World/WorldObjectIdentitySnapshot.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"

namespace yuengine::world {
class WorldObjectIdentityBridge final {
public:
    /**
     * @comment 构造连接一个 WorldInstance 和一个 ObjectRegistry 的 bridge。
     * @param world bridge 查询的 World instance。
     * @param object_registry 用于 handle lifetime operations 的 Object registry。
     * @param desc 输入 bridge descriptor。
     */
    WorldObjectIdentityBridge(WorldInstance &world,
        yuengine::object::ObjectRegistry &object_registry,
        WorldObjectIdentityBridgeDesc desc=WorldObjectIdentityBridgeDesc{});

    /**
     * @comment 将一个 world object id 绑定到一个 object handle，并 acquire 该 handle。
     * @param world_object_id 输入 world object id。
     * @param object_handle 输入 object handle。
     * @return 显式操作结果。
     */
    WorldObjectIdentityResult Bind(WorldObjectId world_object_id,
        yuengine::object::ObjectHandle object_handle);
    /**
     * @comment 通过 ObjectRegistry 校验 existing binding。
     * @param world_object_id 输入 world object id。
     * @return 显式操作状态。
     */
    WorldObjectIdentityStatus Validate(WorldObjectId world_object_id);
    /**
     * @comment 移除一个 binding，并释放已 acquire 的 object handle。
     * @param world_object_id 输入 world object id。
     * @return 显式操作状态。
     */
    WorldObjectIdentityStatus Remove(WorldObjectId world_object_id);
    /**
     * @comment 释放所有 active bindings。
     * @return 显式操作状态。
     */
    WorldObjectIdentityStatus Clear();
    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
     */
    WorldObjectIdentitySnapshot Snapshot() const;

private:
    WorldObjectIdentityStatus RecordFailure(WorldObjectIdentityStatus status);
    WorldObjectIdentityStatus RecordFailure(WorldObjectIdentityStatus status,
        yuengine::object::ObjectStatus object_status);
    void RecordSuccess(yuengine::object::ObjectStatus object_status);
    WorldObjectIdentityStatus ValidateBridgeCapacity() const;
    WorldObjectIdentityStatus MapObjectStatus(yuengine::object::ObjectStatus object_status) const;
    WorldObjectIdentityBinding *FindBindingByWorldObjectId(WorldObjectId world_object_id);
    const WorldObjectIdentityBinding *FindBindingByWorldObjectId(WorldObjectId world_object_id) const;
    WorldObjectIdentityBinding *FindBindingByObjectHandle(yuengine::object::ObjectHandle object_handle);
    WorldObjectIdentityBinding *FindFreeBinding();
    void ClearBinding(WorldObjectIdentityBinding &binding);
    void RecountActiveBindings();

    WorldInstance &world_;
    yuengine::object::ObjectRegistry &object_registry_;
    std::array<WorldObjectIdentityBinding, MAX_WORLD_OBJECT_COUNT> bindings_;
    WorldObjectIdentitySnapshot snapshot_;
};
}
