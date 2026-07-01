// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingBridge.h

#pragma once

#include <array>

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldComponentResourceBinding.h"
#include "YuEngine/World/WorldComponentResourceBindingBridgeDesc.h"
#include "YuEngine/World/WorldComponentResourceBindingResult.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshot.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::world {
class WorldComponentAttachmentBridge;

struct WorldComponentResourceBindingExportResult final {
    WorldComponentResourceBindingStatus status = WorldComponentResourceBindingStatus::Success;
    std::uint32_t required_binding_count = 0U;
    std::uint32_t exported_binding_count = 0U;

    /**
     * @comment 检查 export result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldComponentResourceBindingStatus::Success;
    }
};

class WorldComponentResourceBindingBridge final {
public:
    /**
     * @comment 构造 world component resource binding bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldComponentResourceBindingBridge(
        WorldComponentResourceBindingBridgeDesc desc=WorldComponentResourceBindingBridgeDesc{});

    /**
     * @comment 将一个 existing component attachment tuple 绑定到一个 resource handle。
     * @param attachment_source 调用方持有的 component attachment source。
     * @param resource_registry 调用方持有的 resource registry。
     * @param world_object_id 输入 world object id。
     * @param component_type_id 输入 component type id。
     * @param component_slot_id 输入 component slot id。
     * @param resource_handle 输入 resource handle。
     * @param expected_resource_type 输入 expected resource type。
     * @return 显式操作结果。
     */
    WorldComponentResourceBindingResult Bind(
        const WorldComponentAttachmentBridge *attachment_source,
        yuengine::resource::ResourceRegistry *resource_registry,
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id,
        yuengine::resource::ResourceHandle resource_handle,
        yuengine::resource::ResourceTypeId expected_resource_type);
    /**
     * @comment 按 component attachment tuple 查询一个 component resource binding。
     * @param world_object_id 输入 world object id。
     * @param component_type_id 输入 component type id。
     * @param component_slot_id 输入 component slot id。
     * @return 显式操作结果。
     */
    WorldComponentResourceBindingResult Query(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id);
    /**
     * @comment 移除一个 component resource binding，并释放其 resource handle。
     * @param resource_registry 调用方持有的 resource registry。
     * @param world_object_id 输入 world object id。
     * @param component_type_id 输入 component type id。
     * @param component_slot_id 输入 component slot id。
     * @return 显式操作状态。
     */
    WorldComponentResourceBindingStatus Remove(
        yuengine::resource::ResourceRegistry *resource_registry,
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id);
    /**
     * @comment 释放并清空 deterministic slot order 中的所有 active component resource bindings。
     * @param resource_registry 调用方持有的 resource registry。
     * @return 显式操作状态。
     */
    WorldComponentResourceBindingStatus Clear(yuengine::resource::ResourceRegistry *resource_registry);
    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
     */
    WorldComponentResourceBindingSnapshot Snapshot() const;
    /**
     * @comment 校验 explicit restore preflight 的 destination state。
     * @param required_binding_count Required binding 容量。
     * @return 显式操作状态。
     */
    WorldComponentResourceBindingStatus ValidateRestoreDestination(
        std::uint32_t required_binding_count) const;
    /**
     * @comment 按 deterministic slot order 复制 active component resource binding records。
     * @param output_bindings 调用方持有的 output binding buffer。
     * @param output_capacity 输出 binding buffer capacity。
     * @return active binding 总数。
     */
    std::uint32_t ExportBindings(
        WorldComponentResourceBinding *output_bindings,
        std::uint32_t output_capacity) const;
    /**
     * @comment 按 deterministic slot order 复制 active component resource binding records，容量不足时不写 output buffer。
     * @param output_bindings 调用方持有的 output binding buffer。
     * @param output_capacity 输出 binding buffer capacity。
     * @return 显式 export 结果，包含 required binding count。
     */
    WorldComponentResourceBindingExportResult ExportBindingsChecked(
        WorldComponentResourceBinding *output_bindings,
        std::uint32_t output_capacity) const;

private:
    WorldComponentResourceBindingResult RecordFailureResult(WorldComponentResourceBindingStatus status);
    WorldComponentResourceBindingResult RecordFailureResult(
        WorldComponentResourceBindingStatus status,
        yuengine::resource::ResourceStatus resource_status);
    WorldComponentResourceBindingStatus RecordFailure(WorldComponentResourceBindingStatus status);
    WorldComponentResourceBindingStatus RecordFailure(
        WorldComponentResourceBindingStatus status,
        yuengine::resource::ResourceStatus resource_status);
    void RecordSuccess(yuengine::resource::ResourceStatus resource_status);
    WorldComponentResourceBindingStatus ValidateBridgeCapacity() const;
    WorldComponentResourceBindingStatus ValidateAttachmentTuple(
        const WorldComponentAttachmentBridge *attachment_source,
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id) const;
    WorldComponentResourceBindingStatus MapAcquireStatus(yuengine::resource::ResourceStatus resource_status) const;
    WorldComponentResourceBinding *FindBinding(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id);
    const WorldComponentResourceBinding *FindBinding(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id) const;
    WorldComponentResourceBinding *FindFreeBinding();
    void ClearBinding(WorldComponentResourceBinding &binding);
    void RecountActiveBindings();

    std::array<WorldComponentResourceBinding, MAX_WORLD_OBJECT_COUNT> bindings_;
    WorldComponentResourceBindingSnapshot snapshot_;
};
}
