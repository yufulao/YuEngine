// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldSceneAssemblyBridge.cpp

#include "YuEngine/World/WorldSceneAssemblyBridge.h"

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/World/WorldComponentAttachmentBridge.h"
#include "YuEngine/World/WorldComponentAttachmentResult.h"
#include "YuEngine/World/WorldComponentResourceBindingBridge.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreBridge.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreBridgeDesc.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreResult.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceStatus;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity) {
    if (requested_capacity > MAX_WORLD_OBJECT_COUNT) {
        return MAX_WORLD_OBJECT_COUNT;
    }

    return requested_capacity;
}
}

WorldSceneAssemblyBridge::WorldSceneAssemblyBridge(WorldSceneAssemblyBridgeDesc desc)
    : attachment_capacity_(ClampCapacity(desc.attachment_capacity)),
      binding_capacity_(ClampCapacity(desc.binding_capacity)),
      snapshot_{
          ClampCapacity(desc.attachment_capacity),
          ClampCapacity(desc.binding_capacity),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          ResourceStatus::Success,
          WorldComponentAttachmentStatus::Success,
          WorldComponentResourceBindingStatus::Success,
          WorldComponentResourceBindingRestoreStatus::Success,
          WorldSceneAssemblyStatus::Success} {
    if ((desc.attachment_capacity == 0U) || (desc.binding_capacity == 0U)) {
        snapshot_.last_status = WorldSceneAssemblyStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldSceneAssemblyResult WorldSceneAssemblyBridge::Restore(
    WorldComponentAttachmentBridge *attachment_destination,
    WorldComponentResourceBindingBridge *binding_destination,
    ResourceRegistry *resource_registry,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count) {
    ++snapshot_.assembly_attempt_count;

    const WorldSceneAssemblyStatus bridge_status = ValidateBridgeCapacity();
    if (bridge_status != WorldSceneAssemblyStatus::Success) {
        return RecordFailure(bridge_status);
    }

    if (attachment_destination == nullptr) {
        return RecordFailure(WorldSceneAssemblyStatus::InvalidAttachmentDestination);
    }

    if (binding_destination == nullptr) {
        return RecordFailure(WorldSceneAssemblyStatus::InvalidBindingDestination);
    }

    if (resource_registry == nullptr) {
        return RecordFailure(WorldSceneAssemblyStatus::InvalidResourceRegistry);
    }

    if (input_attachments == nullptr) {
        return RecordFailure(WorldSceneAssemblyStatus::InvalidAttachmentInput);
    }

    if (input_bindings == nullptr) {
        return RecordFailure(WorldSceneAssemblyStatus::InvalidBindingInput);
    }

    WorldComponentAttachmentStatus attachment_status = WorldComponentAttachmentStatus::Success;
    const WorldSceneAssemblyStatus attachment_destination_status = ValidateAttachmentDestination(
        *attachment_destination,
        input_attachment_count,
        &attachment_status);
    if (attachment_destination_status != WorldSceneAssemblyStatus::Success) {
        return RecordFailure(attachment_destination_status, attachment_status);
    }

    WorldComponentResourceBindingStatus binding_status = WorldComponentResourceBindingStatus::Success;
    const WorldSceneAssemblyStatus binding_destination_status = ValidateBindingDestination(
        *binding_destination,
        input_binding_count,
        &binding_status);
    if (binding_destination_status != WorldSceneAssemblyStatus::Success) {
        return RecordFailure(binding_destination_status, binding_status);
    }

    if (input_attachment_count > attachment_capacity_) {
        return RecordRejectedFailure(WorldSceneAssemblyStatus::AttachmentCapacityExceeded);
    }

    if (input_binding_count > binding_capacity_) {
        return RecordRejectedFailure(WorldSceneAssemblyStatus::BindingCapacityExceeded);
    }

    const WorldSceneAssemblyStatus attachment_record_status =
        ValidateAttachmentRecords(input_attachments, input_attachment_count);
    if (attachment_record_status != WorldSceneAssemblyStatus::Success) {
        return RecordRejectedFailure(attachment_record_status);
    }

    ResourceStatus resource_status = ResourceStatus::Success;
    const WorldSceneAssemblyStatus binding_record_status = ValidateBindingRecords(
        *resource_registry,
        input_attachments,
        input_attachment_count,
        input_bindings,
        input_binding_count,
        &resource_status);
    if (binding_record_status != WorldSceneAssemblyStatus::Success) {
        return RecordRejectedFailure(binding_record_status, resource_status);
    }

    WorldSceneAssemblyState state{};
    state.input_attachment_count = input_attachment_count;
    state.input_binding_count = input_binding_count;
    attachment_status = ApplyAttachments(
        attachment_destination,
        input_attachments,
        input_attachment_count,
        &state);
    if (attachment_status != WorldComponentAttachmentStatus::Success) {
        return RecordFailure(WorldSceneAssemblyStatus::AttachmentApplyFailed, attachment_status);
    }

    return RestoreBindings(
        attachment_destination,
        binding_destination,
        resource_registry,
        input_bindings,
        input_binding_count,
        &state);
}

WorldSceneAssemblySnapshot WorldSceneAssemblyBridge::Snapshot() const {
    return snapshot_;
}

WorldSceneAssemblyResult WorldSceneAssemblyBridge::RecordFailure(WorldSceneAssemblyStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_attachment_status = WorldComponentAttachmentStatus::Success;
    snapshot_.last_binding_status = WorldComponentResourceBindingStatus::Success;
    snapshot_.last_binding_restore_status = WorldComponentResourceBindingRestoreStatus::Success;
    snapshot_.last_resource_status = ResourceStatus::Success;
    return WorldSceneAssemblyResult::Failure(status);
}

WorldSceneAssemblyResult WorldSceneAssemblyBridge::RecordFailure(
    WorldSceneAssemblyStatus status,
    WorldComponentAttachmentStatus attachment_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_attachment_status = attachment_status;
    snapshot_.last_binding_status = WorldComponentResourceBindingStatus::Success;
    snapshot_.last_binding_restore_status = WorldComponentResourceBindingRestoreStatus::Success;
    snapshot_.last_resource_status = ResourceStatus::Success;
    return WorldSceneAssemblyResult::Failure(
        status,
        attachment_status,
        WorldComponentResourceBindingStatus::Success,
        WorldComponentResourceBindingRestoreStatus::Success,
        ResourceStatus::Success);
}

WorldSceneAssemblyResult WorldSceneAssemblyBridge::RecordFailure(
    WorldSceneAssemblyStatus status,
    WorldComponentResourceBindingStatus binding_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_attachment_status = WorldComponentAttachmentStatus::Success;
    snapshot_.last_binding_status = binding_status;
    snapshot_.last_binding_restore_status = WorldComponentResourceBindingRestoreStatus::Success;
    snapshot_.last_resource_status = ResourceStatus::Success;
    return WorldSceneAssemblyResult::Failure(
        status,
        WorldComponentAttachmentStatus::Success,
        binding_status,
        WorldComponentResourceBindingRestoreStatus::Success,
        ResourceStatus::Success);
}

WorldSceneAssemblyResult WorldSceneAssemblyBridge::RecordFailure(
    WorldSceneAssemblyStatus status,
    WorldComponentResourceBindingRestoreStatus binding_restore_status,
    WorldComponentResourceBindingStatus binding_status,
    ResourceStatus resource_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_attachment_status = WorldComponentAttachmentStatus::Success;
    snapshot_.last_binding_status = binding_status;
    snapshot_.last_binding_restore_status = binding_restore_status;
    snapshot_.last_resource_status = resource_status;
    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::RollbackFailed) {
        ++snapshot_.rollback_count;
    }

    return WorldSceneAssemblyResult::Failure(
        status,
        WorldComponentAttachmentStatus::Success,
        binding_status,
        binding_restore_status,
        resource_status);
}

WorldSceneAssemblyResult WorldSceneAssemblyBridge::RecordFailure(
    WorldSceneAssemblyStatus status,
    ResourceStatus resource_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_attachment_status = WorldComponentAttachmentStatus::Success;
    snapshot_.last_binding_status = WorldComponentResourceBindingStatus::Success;
    snapshot_.last_binding_restore_status = WorldComponentResourceBindingRestoreStatus::Success;
    snapshot_.last_resource_status = resource_status;
    return WorldSceneAssemblyResult::Failure(
        status,
        WorldComponentAttachmentStatus::Success,
        WorldComponentResourceBindingStatus::Success,
        WorldComponentResourceBindingRestoreStatus::Success,
        resource_status);
}

WorldSceneAssemblyResult WorldSceneAssemblyBridge::RecordRejectedFailure(
    WorldSceneAssemblyStatus status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(status);
}

WorldSceneAssemblyResult WorldSceneAssemblyBridge::RecordRejectedFailure(
    WorldSceneAssemblyStatus status,
    ResourceStatus resource_status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(status, resource_status);
}

WorldSceneAssemblyResult WorldSceneAssemblyBridge::RecordSuccess(
    const WorldSceneAssemblyState &state) {
    snapshot_.restored_attachment_count += state.restored_attachment_count;
    snapshot_.restored_binding_count += state.restored_binding_count;
    snapshot_.last_status = WorldSceneAssemblyStatus::Success;
    snapshot_.last_attachment_status = WorldComponentAttachmentStatus::Success;
    snapshot_.last_binding_status = WorldComponentResourceBindingStatus::Success;
    snapshot_.last_binding_restore_status = WorldComponentResourceBindingRestoreStatus::Success;
    snapshot_.last_resource_status = ResourceStatus::Success;
    return WorldSceneAssemblyResult::Success(state);
}

WorldSceneAssemblyStatus WorldSceneAssemblyBridge::ValidateBridgeCapacity() const {
    if ((attachment_capacity_ == 0U) || (binding_capacity_ == 0U)) {
        return WorldSceneAssemblyStatus::InvalidBridgeCapacity;
    }

    return WorldSceneAssemblyStatus::Success;
}

WorldSceneAssemblyStatus WorldSceneAssemblyBridge::ValidateAttachmentDestination(
    const WorldComponentAttachmentBridge &attachment_destination,
    std::uint32_t input_attachment_count,
    WorldComponentAttachmentStatus *out_attachment_status) const {
    const WorldComponentAttachmentStatus attachment_status =
        attachment_destination.ValidateRestoreDestination(input_attachment_count);
    if (out_attachment_status != nullptr) {
        *out_attachment_status = attachment_status;
    }

    if (attachment_status == WorldComponentAttachmentStatus::Success) {
        return WorldSceneAssemblyStatus::Success;
    }

    return MapAttachmentDestinationStatus(attachment_status);
}

WorldSceneAssemblyStatus WorldSceneAssemblyBridge::ValidateBindingDestination(
    const WorldComponentResourceBindingBridge &binding_destination,
    std::uint32_t input_binding_count,
    WorldComponentResourceBindingStatus *out_binding_status) const {
    const WorldComponentResourceBindingStatus binding_status =
        binding_destination.ValidateRestoreDestination(input_binding_count);
    if (out_binding_status != nullptr) {
        *out_binding_status = binding_status;
    }

    if (binding_status == WorldComponentResourceBindingStatus::Success) {
        return WorldSceneAssemblyStatus::Success;
    }

    return MapBindingDestinationStatus(binding_status);
}

WorldSceneAssemblyStatus WorldSceneAssemblyBridge::ValidateAttachmentRecords(
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < input_attachment_count) {
        const WorldSceneAssemblyStatus status = ValidateAttachmentRecord(input_attachments, record_index);
        if (status != WorldSceneAssemblyStatus::Success) {
            return status;
        }

        ++record_index;
    }

    return WorldSceneAssemblyStatus::Success;
}

WorldSceneAssemblyStatus WorldSceneAssemblyBridge::ValidateAttachmentRecord(
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t record_index) const {
    const WorldComponentAttachmentSnapshotRecord &attachment = input_attachments[record_index];
    if (!attachment.world_object_id.IsValid()) {
        return WorldSceneAssemblyStatus::InvalidWorldObjectId;
    }

    if (!attachment.component_type_id.IsValid()) {
        return WorldSceneAssemblyStatus::InvalidComponentTypeId;
    }

    if (!attachment.component_slot_id.IsValid()) {
        return WorldSceneAssemblyStatus::InvalidComponentSlotId;
    }

    if (HasDuplicateAttachmentInput(input_attachments, record_index)) {
        return WorldSceneAssemblyStatus::DuplicateAttachmentInput;
    }

    return WorldSceneAssemblyStatus::Success;
}

WorldSceneAssemblyStatus WorldSceneAssemblyBridge::ValidateBindingRecords(
    const ResourceRegistry &resource_registry,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count,
    ResourceStatus *out_resource_status) const {
    std::uint32_t record_index = 0U;
    while (record_index < input_binding_count) {
        const WorldSceneAssemblyStatus status = ValidateBindingRecord(
            resource_registry,
            input_attachments,
            input_attachment_count,
            input_bindings,
            record_index,
            out_resource_status);
        if (status != WorldSceneAssemblyStatus::Success) {
            return status;
        }

        ++record_index;
    }

    return WorldSceneAssemblyStatus::Success;
}

WorldSceneAssemblyStatus WorldSceneAssemblyBridge::ValidateBindingRecord(
    const ResourceRegistry &resource_registry,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t record_index,
    ResourceStatus *out_resource_status) const {
    const WorldComponentResourceBindingSnapshotRecord &binding = input_bindings[record_index];
    if (!binding.world_object_id.IsValid()) {
        return WorldSceneAssemblyStatus::InvalidWorldObjectId;
    }

    if (!binding.component_type_id.IsValid()) {
        return WorldSceneAssemblyStatus::InvalidComponentTypeId;
    }

    if (!binding.component_slot_id.IsValid()) {
        return WorldSceneAssemblyStatus::InvalidComponentSlotId;
    }

    if (!binding.resource_handle.IsValid()) {
        if (out_resource_status != nullptr) {
            *out_resource_status = ResourceStatus::InvalidHandle;
        }

        return WorldSceneAssemblyStatus::InvalidResourceHandle;
    }

    if (!binding.expected_resource_type.IsValid()) {
        return WorldSceneAssemblyStatus::InvalidResourceTypeId;
    }

    if (HasDuplicateBindingInput(input_bindings, record_index)) {
        return WorldSceneAssemblyStatus::DuplicateBindingInput;
    }

    if (!HasAttachmentTuple(input_attachments, input_attachment_count, binding)) {
        return WorldSceneAssemblyStatus::MissingAttachment;
    }

    const std::uint32_t projected_acquire_count = CountProjectedAcquire(input_bindings, record_index);
    const ResourceStatus resource_status = resource_registry.ValidateAcquire(
        binding.resource_handle,
        binding.expected_resource_type,
        projected_acquire_count);
    if (resource_status != ResourceStatus::Success) {
        if (out_resource_status != nullptr) {
            *out_resource_status = resource_status;
        }

        return MapResourceStatus(resource_status);
    }

    return WorldSceneAssemblyStatus::Success;
}

bool WorldSceneAssemblyBridge::HasDuplicateAttachmentInput(
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t record_index) const {
    const WorldComponentAttachmentSnapshotRecord &attachment = input_attachments[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldComponentAttachmentSnapshotRecord &compare_attachment = input_attachments[compare_index];
        if (compare_attachment.world_object_id.value != attachment.world_object_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_attachment.component_type_id.value == attachment.component_type_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneAssemblyBridge::HasDuplicateBindingInput(
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t record_index) const {
    const WorldComponentResourceBindingSnapshotRecord &binding = input_bindings[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldComponentResourceBindingSnapshotRecord &compare_binding = input_bindings[compare_index];
        if (compare_binding.world_object_id.value != binding.world_object_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_binding.component_type_id.value != binding.component_type_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_binding.component_slot_id.value == binding.component_slot_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneAssemblyBridge::HasAttachmentTuple(
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord &binding) const {
    std::uint32_t attachment_index = 0U;
    while (attachment_index < input_attachment_count) {
        const WorldComponentAttachmentSnapshotRecord &attachment = input_attachments[attachment_index];
        if (attachment.world_object_id.value != binding.world_object_id.value) {
            ++attachment_index;
            continue;
        }

        if (attachment.component_type_id.value != binding.component_type_id.value) {
            ++attachment_index;
            continue;
        }

        if (attachment.component_slot_id.value == binding.component_slot_id.value) {
            return true;
        }

        ++attachment_index;
    }

    return false;
}

std::uint32_t WorldSceneAssemblyBridge::CountProjectedAcquire(
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t record_index) const {
    const WorldComponentResourceBindingSnapshotRecord &binding = input_bindings[record_index];
    std::uint32_t projected_acquire_count = 0U;
    std::uint32_t compare_index = 0U;
    while (compare_index <= record_index) {
        const WorldComponentResourceBindingSnapshotRecord &compare_binding = input_bindings[compare_index];
        if (compare_binding.resource_handle.slot != binding.resource_handle.slot) {
            ++compare_index;
            continue;
        }

        if (compare_binding.resource_handle.generation == binding.resource_handle.generation) {
            ++projected_acquire_count;
        }

        ++compare_index;
    }

    return projected_acquire_count;
}

WorldSceneAssemblyStatus WorldSceneAssemblyBridge::MapAttachmentDestinationStatus(
    WorldComponentAttachmentStatus attachment_status) const {
    if (attachment_status == WorldComponentAttachmentStatus::InvalidBridgeCapacity) {
        return WorldSceneAssemblyStatus::InvalidBridgeCapacity;
    }

    if (attachment_status == WorldComponentAttachmentStatus::CapacityExceeded) {
        return WorldSceneAssemblyStatus::AttachmentCapacityExceeded;
    }

    if (attachment_status == WorldComponentAttachmentStatus::DuplicateAttachment) {
        return WorldSceneAssemblyStatus::DestinationNotEmpty;
    }

    return WorldSceneAssemblyStatus::AttachmentApplyFailed;
}

WorldSceneAssemblyStatus WorldSceneAssemblyBridge::MapBindingDestinationStatus(
    WorldComponentResourceBindingStatus binding_status) const {
    if (binding_status == WorldComponentResourceBindingStatus::InvalidBridgeCapacity) {
        return WorldSceneAssemblyStatus::InvalidBridgeCapacity;
    }

    if (binding_status == WorldComponentResourceBindingStatus::CapacityExceeded) {
        return WorldSceneAssemblyStatus::BindingCapacityExceeded;
    }

    if (binding_status == WorldComponentResourceBindingStatus::DuplicateComponentBinding) {
        return WorldSceneAssemblyStatus::DestinationNotEmpty;
    }

    return WorldSceneAssemblyStatus::BindingRestoreFailed;
}

WorldSceneAssemblyStatus WorldSceneAssemblyBridge::MapResourceStatus(ResourceStatus resource_status) const {
    if (resource_status == ResourceStatus::InvalidHandle) {
        return WorldSceneAssemblyStatus::InvalidResourceHandle;
    }

    if (resource_status == ResourceStatus::GenerationMismatch) {
        return WorldSceneAssemblyStatus::StaleResourceHandle;
    }

    if (resource_status == ResourceStatus::TypeMismatch) {
        return WorldSceneAssemblyStatus::ResourceTypeMismatch;
    }

    if (resource_status == ResourceStatus::ReferenceCountOverflow) {
        return WorldSceneAssemblyStatus::ResourceAcquireWouldOverflow;
    }

    return WorldSceneAssemblyStatus::ResourceAcquireFailed;
}

WorldSceneAssemblyStatus WorldSceneAssemblyBridge::MapBindingRestoreStatus(
    WorldComponentResourceBindingRestoreStatus binding_restore_status) const {
    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::InvalidBridgeCapacity) {
        return WorldSceneAssemblyStatus::InvalidBridgeCapacity;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::InvalidDestinationBridge) {
        return WorldSceneAssemblyStatus::InvalidBindingDestination;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::InvalidAttachmentSource) {
        return WorldSceneAssemblyStatus::InvalidAttachmentDestination;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::InvalidResourceRegistry) {
        return WorldSceneAssemblyStatus::InvalidResourceRegistry;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::InvalidInput) {
        return WorldSceneAssemblyStatus::InvalidBindingInput;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::InputCountExceeded) {
        return WorldSceneAssemblyStatus::BindingCapacityExceeded;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::InvalidWorldObjectId) {
        return WorldSceneAssemblyStatus::InvalidWorldObjectId;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::InvalidComponentTypeId) {
        return WorldSceneAssemblyStatus::InvalidComponentTypeId;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::InvalidComponentSlotId) {
        return WorldSceneAssemblyStatus::InvalidComponentSlotId;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::MissingAttachment) {
        return WorldSceneAssemblyStatus::MissingAttachment;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::InvalidResourceHandle) {
        return WorldSceneAssemblyStatus::InvalidResourceHandle;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::StaleResourceHandle) {
        return WorldSceneAssemblyStatus::StaleResourceHandle;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::InvalidResourceTypeId) {
        return WorldSceneAssemblyStatus::InvalidResourceTypeId;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::ResourceTypeMismatch) {
        return WorldSceneAssemblyStatus::ResourceTypeMismatch;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::ResourceAcquireWouldOverflow) {
        return WorldSceneAssemblyStatus::ResourceAcquireWouldOverflow;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::DuplicateInputBinding) {
        return WorldSceneAssemblyStatus::DuplicateBindingInput;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::DestinationCapacityExceeded) {
        return WorldSceneAssemblyStatus::BindingCapacityExceeded;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::DestinationNotEmpty) {
        return WorldSceneAssemblyStatus::DestinationNotEmpty;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::ResourceAcquireFailed) {
        return WorldSceneAssemblyStatus::ResourceAcquireFailed;
    }

    if (binding_restore_status == WorldComponentResourceBindingRestoreStatus::RollbackFailed) {
        return WorldSceneAssemblyStatus::RollbackFailed;
    }

    return WorldSceneAssemblyStatus::BindingRestoreFailed;
}

WorldComponentAttachmentStatus WorldSceneAssemblyBridge::ApplyAttachments(
    WorldComponentAttachmentBridge *attachment_destination,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    WorldSceneAssemblyState *state) const {
    std::uint32_t record_index = 0U;
    while (record_index < input_attachment_count) {
        const WorldComponentAttachmentSnapshotRecord &attachment = input_attachments[record_index];
        const WorldComponentAttachmentResult attachment_result = attachment_destination->Add(
            attachment.world_object_id,
            attachment.component_type_id,
            attachment.component_slot_id);
        if (!attachment_result.Succeeded()) {
            return attachment_result.status;
        }

        if (state != nullptr) {
            ++state->restored_attachment_count;
        }

        ++record_index;
    }

    return WorldComponentAttachmentStatus::Success;
}

void WorldSceneAssemblyBridge::BuildBindingInputs(
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count,
    std::array<WorldComponentResourceBinding, MAX_WORLD_OBJECT_COUNT> *output_bindings) const {
    if (output_bindings == nullptr) {
        return;
    }

    std::uint32_t record_index = 0U;
    while (record_index < input_binding_count) {
        const WorldComponentResourceBindingSnapshotRecord &input_binding = input_bindings[record_index];
        WorldComponentResourceBinding &output_binding = (*output_bindings)[record_index];
        output_binding.world_object_id = input_binding.world_object_id;
        output_binding.component_type_id = input_binding.component_type_id;
        output_binding.component_slot_id = input_binding.component_slot_id;
        output_binding.resource_handle = input_binding.resource_handle;
        output_binding.expected_resource_type = input_binding.expected_resource_type;
        output_binding.is_bound = true;
        output_binding.is_acquired = false;
        ++record_index;
    }
}

WorldSceneAssemblyResult WorldSceneAssemblyBridge::RestoreBindings(
    WorldComponentAttachmentBridge *attachment_destination,
    WorldComponentResourceBindingBridge *binding_destination,
    ResourceRegistry *resource_registry,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count,
    WorldSceneAssemblyState *state) {
    if (state == nullptr) {
        return RecordFailure(WorldSceneAssemblyStatus::InvalidBindingInput);
    }

    std::array<WorldComponentResourceBinding, MAX_WORLD_OBJECT_COUNT> binding_inputs{};
    BuildBindingInputs(input_bindings, input_binding_count, &binding_inputs);

    WorldComponentResourceBindingRestoreBridgeDesc restore_desc{};
    restore_desc.binding_capacity = binding_capacity_;
    WorldComponentResourceBindingRestoreBridge restore_bridge{restore_desc};
    const WorldComponentResourceBindingRestoreResult restore_result = restore_bridge.Restore(
        binding_destination,
        attachment_destination,
        resource_registry,
        binding_inputs.data(),
        input_binding_count);
    if (!restore_result.Succeeded()) {
        const WorldSceneAssemblyStatus status = MapBindingRestoreStatus(restore_result.status);
        return RecordFailure(
            status,
            restore_result.status,
            restore_result.binding_status,
            restore_result.resource_status);
    }

    if (state != nullptr) {
        state->restored_binding_count = restore_result.state.restored_binding_count;
    }

    return RecordSuccess(*state);
}
}
