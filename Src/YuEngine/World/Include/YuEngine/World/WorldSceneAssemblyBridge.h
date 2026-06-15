// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyBridge.h

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
     * @comment Constructs a world scene assembly bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldSceneAssemblyBridge(
        WorldSceneAssemblyBridgeDesc desc=WorldSceneAssemblyBridgeDesc{});

    /**
     * @comment Restores caller-owned sidecar records after full assembly preflight.
     * @param attachment_destination Caller-owned attachment destination.
     * @param binding_destination Caller-owned binding destination.
     * @param resource_registry Caller-owned resource registry.
     * @param input_attachments Caller-owned attachment input records.
     * @param input_attachment_count Input attachment record count.
     * @param input_bindings Caller-owned binding input records.
     * @param input_binding_count Input binding record count.
     * @return Explicit operation result.
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
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
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
