// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyBridge.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldComponentResourceBinding.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldSceneAssemblyBridgeDesc.h"
#include "YuEngine/World/WorldSceneAssemblyResult.h"
#include "YuEngine/World/WorldSceneAssemblySnapshot.h"
#include "YuEngine/World/WorldSceneAssemblyStatus.h"

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::world {
class WorldComponentAttachmentBridge;
class WorldComponentResourceBindingBridge;

class WorldSceneAssemblyBridge final {
public:
    /**
     * @comment 构造 world scene assembly bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldSceneAssemblyBridge(
        WorldSceneAssemblyBridgeDesc desc=WorldSceneAssemblyBridgeDesc{});

    /**
     * @comment 在 full assembly preflight 后恢复调用方持有的 sidecar records。
     * @param attachment_destination 调用方持有的 attachment destination。
     * @param binding_destination 调用方持有的 binding destination。
     * @param resource_registry 调用方持有的 resource registry。
     * @param input_attachments 调用方持有的 attachment input records。
     * @param input_attachment_count 输入 attachment record count。
     * @param input_bindings 调用方持有的 binding input records。
     * @param input_binding_count 输入 binding record count。
     * @return 显式操作结果。
     */
    WorldSceneAssemblyResult Restore(
        WorldComponentAttachmentBridge *attachment_destination,
        WorldComponentResourceBindingBridge *binding_destination,
        yuengine::resource::ResourceRegistry *resource_registry,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count);
    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
     */
    WorldSceneAssemblySnapshot Snapshot() const;

private:
    WorldSceneAssemblyResult RecordFailure(WorldSceneAssemblyStatus status);
    WorldSceneAssemblyResult RecordFailure(
        WorldSceneAssemblyStatus status,
        WorldComponentAttachmentStatus attachment_status);
    WorldSceneAssemblyResult RecordFailure(
        WorldSceneAssemblyStatus status,
        WorldComponentResourceBindingStatus binding_status);
    WorldSceneAssemblyResult RecordFailure(
        WorldSceneAssemblyStatus status,
        WorldComponentResourceBindingRestoreStatus binding_restore_status,
        WorldComponentResourceBindingStatus binding_status,
        yuengine::resource::ResourceStatus resource_status);
    WorldSceneAssemblyResult RecordFailure(
        WorldSceneAssemblyStatus status,
        yuengine::resource::ResourceStatus resource_status);
    WorldSceneAssemblyResult RecordRejectedFailure(WorldSceneAssemblyStatus status);
    WorldSceneAssemblyResult RecordRejectedFailure(
        WorldSceneAssemblyStatus status,
        yuengine::resource::ResourceStatus resource_status);
    WorldSceneAssemblyResult RecordSuccess(const WorldSceneAssemblyState &state);
    void ClearCapacityEntry();
    void StoreCapacityEntry(
        std::uint32_t record_index,
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id,
        std::uint32_t destination_capacity,
        std::uint32_t destination_count,
        std::uint32_t required_count,
        WorldSceneAssemblyResult *result);
    void StoreAttachmentCapacityEntry(
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        std::uint32_t record_index,
        std::uint32_t destination_capacity,
        std::uint32_t destination_count,
        WorldSceneAssemblyResult *result);
    void StoreBindingCapacityEntry(
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count,
        std::uint32_t record_index,
        std::uint32_t destination_capacity,
        std::uint32_t destination_count,
        WorldSceneAssemblyResult *result);
    WorldSceneAssemblyStatus ValidateBridgeCapacity() const;
    WorldSceneAssemblyStatus ValidateAttachmentDestination(
        const WorldComponentAttachmentBridge &attachment_destination,
        std::uint32_t input_attachment_count,
        WorldComponentAttachmentStatus *out_attachment_status) const;
    WorldSceneAssemblyStatus ValidateBindingDestination(
        const WorldComponentResourceBindingBridge &binding_destination,
        std::uint32_t input_binding_count,
        WorldComponentResourceBindingStatus *out_binding_status) const;
    WorldSceneAssemblyStatus ValidateAttachmentRecords(
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count) const;
    WorldSceneAssemblyStatus ValidateAttachmentRecord(
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t record_index) const;
    WorldSceneAssemblyStatus ValidateBindingRecords(
        const yuengine::resource::ResourceRegistry &resource_registry,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count,
        yuengine::resource::ResourceStatus *out_resource_status) const;
    WorldSceneAssemblyStatus ValidateBindingRecord(
        const yuengine::resource::ResourceRegistry &resource_registry,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t record_index,
        yuengine::resource::ResourceStatus *out_resource_status) const;
    bool HasDuplicateAttachmentInput(
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t record_index) const;
    bool HasDuplicateBindingInput(
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t record_index) const;
    bool HasAttachmentTuple(
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord &binding) const;
    std::uint32_t CountProjectedAcquire(
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t record_index) const;
    WorldSceneAssemblyStatus MapAttachmentDestinationStatus(
        WorldComponentAttachmentStatus attachment_status) const;
    WorldSceneAssemblyStatus MapBindingDestinationStatus(
        WorldComponentResourceBindingStatus binding_status) const;
    WorldSceneAssemblyStatus MapResourceStatus(
        yuengine::resource::ResourceStatus resource_status) const;
    WorldSceneAssemblyStatus MapBindingRestoreStatus(
        WorldComponentResourceBindingRestoreStatus binding_restore_status) const;
    WorldComponentAttachmentStatus ApplyAttachments(
        WorldComponentAttachmentBridge *attachment_destination,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        WorldSceneAssemblyState *state) const;
    void BuildBindingInputs(
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count,
        std::array<WorldComponentResourceBinding, MAX_WORLD_OBJECT_COUNT> *output_bindings) const;
    WorldSceneAssemblyResult RestoreBindings(
        WorldComponentAttachmentBridge *attachment_destination,
        WorldComponentResourceBindingBridge *binding_destination,
        yuengine::resource::ResourceRegistry *resource_registry,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count,
        WorldSceneAssemblyState *state);

    std::uint32_t attachment_capacity_;
    std::uint32_t binding_capacity_;
    WorldSceneAssemblySnapshot snapshot_;
};
}
