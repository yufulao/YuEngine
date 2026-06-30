// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldComponentResourceBindingRestoreBridge.cpp

#include "YuEngine/World/WorldComponentResourceBindingRestoreBridge.h"

#include <array>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/World/WorldComponentAttachmentBridge.h"
#include "YuEngine/World/WorldComponentResourceBindingBridge.h"
#include "YuEngine/World/WorldConstants.h"

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

WorldComponentResourceBindingRestoreBridge::WorldComponentResourceBindingRestoreBridge(
    WorldComponentResourceBindingRestoreBridgeDesc desc)
    : binding_capacity_(ClampCapacity(desc.binding_capacity)),
      snapshot_{
          ClampCapacity(desc.binding_capacity),
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          ResourceStatus::Success,
          WorldComponentResourceBindingStatus::Success,
          WorldComponentResourceBindingRestoreStatus::Success} {
    if (desc.binding_capacity == 0U) {
        snapshot_.last_status = WorldComponentResourceBindingRestoreStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldComponentResourceBindingRestoreResult WorldComponentResourceBindingRestoreBridge::Restore(
    WorldComponentResourceBindingBridge *destination_bridge,
    const WorldComponentAttachmentBridge *attachment_source,
    ResourceRegistry *resource_registry,
    const WorldComponentResourceBinding *input_bindings,
    std::uint32_t input_binding_count) {
    ++snapshot_.restore_attempt_count;

    const WorldComponentResourceBindingRestoreStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentResourceBindingRestoreStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (destination_bridge == nullptr) {
        return RecordFailure(WorldComponentResourceBindingRestoreStatus::InvalidDestinationBridge);
    }

    if (attachment_source == nullptr) {
        return RecordFailure(WorldComponentResourceBindingRestoreStatus::InvalidAttachmentSource);
    }

    if (resource_registry == nullptr) {
        return RecordFailure(WorldComponentResourceBindingRestoreStatus::InvalidResourceRegistry);
    }

    if (input_bindings == nullptr) {
        return RecordFailure(WorldComponentResourceBindingRestoreStatus::InvalidInput);
    }

    const WorldComponentResourceBindingRestoreStatus destination_status =
        ValidateDestination(*destination_bridge, input_binding_count);
    if (destination_status != WorldComponentResourceBindingRestoreStatus::Success) {
        return RecordFailure(destination_status);
    }

    WorldComponentResourceBindingRestoreState state{};
    state.input_binding_count = input_binding_count;
    if (input_binding_count > binding_capacity_) {
        state.rejected_binding_count = input_binding_count;
        return RecordRejectedFailure(
            WorldComponentResourceBindingRestoreStatus::InputCountExceeded,
            state);
    }

    ResourceStatus resource_status = ResourceStatus::Success;
    const WorldComponentResourceBindingRestoreStatus record_status = ValidateRecords(
        *attachment_source,
        *resource_registry,
        input_bindings,
        input_binding_count,
        &resource_status,
        &state);
    if (record_status != WorldComponentResourceBindingRestoreStatus::Success) {
        return RecordRejectedFailure(record_status, resource_status, state);
    }

    if (input_binding_count == 0U) {
        return RecordSuccess(state);
    }

    std::uint32_t record_index = 0U;
    while (record_index < input_binding_count) {
        const WorldComponentResourceBinding &binding = input_bindings[record_index];
        const WorldComponentResourceBindingResult binding_result = destination_bridge->Bind(
            attachment_source,
            resource_registry,
            binding.world_object_id,
            binding.component_type_id,
            binding.component_slot_id,
            binding.resource_handle,
            binding.expected_resource_type);
        if (!binding_result.Succeeded()) {
            return RollbackAndRecordApplyFailure(
                destination_bridge,
                resource_registry,
                state,
                binding_result);
        }

        ++state.restored_binding_count;
        ++record_index;
    }

    return RecordSuccess(state);
}

WorldComponentResourceBindingRestoreSnapshot WorldComponentResourceBindingRestoreBridge::Snapshot() const {
    return snapshot_;
}

WorldComponentResourceBindingRestoreResult WorldComponentResourceBindingRestoreBridge::RecordFailure(
    WorldComponentResourceBindingRestoreStatus status) {
    return RecordFailure(
        status,
        WorldComponentResourceBindingStatus::Success,
        ResourceStatus::Success);
}

WorldComponentResourceBindingRestoreResult WorldComponentResourceBindingRestoreBridge::RecordFailure(
    WorldComponentResourceBindingRestoreStatus status,
    WorldComponentResourceBindingStatus binding_status) {
    return RecordFailure(status, binding_status, ResourceStatus::Success);
}

WorldComponentResourceBindingRestoreResult WorldComponentResourceBindingRestoreBridge::RecordFailure(
    WorldComponentResourceBindingRestoreStatus status,
    ResourceStatus resource_status) {
    return RecordFailure(status, WorldComponentResourceBindingStatus::Success, resource_status);
}

WorldComponentResourceBindingRestoreResult WorldComponentResourceBindingRestoreBridge::RecordFailure(
    WorldComponentResourceBindingRestoreStatus status,
    WorldComponentResourceBindingStatus binding_status,
    ResourceStatus resource_status) {
    return RecordFailure(
        status,
        binding_status,
        resource_status,
        WorldComponentResourceBindingRestoreState{});
}

WorldComponentResourceBindingRestoreResult WorldComponentResourceBindingRestoreBridge::RecordFailure(
    WorldComponentResourceBindingRestoreStatus status,
    WorldComponentResourceBindingStatus binding_status,
    ResourceStatus resource_status,
    const WorldComponentResourceBindingRestoreState &state) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_binding_status = binding_status;
    snapshot_.last_resource_status = resource_status;
    return WorldComponentResourceBindingRestoreResult::Failure(
        status,
        binding_status,
        resource_status,
        state);
}

WorldComponentResourceBindingRestoreResult WorldComponentResourceBindingRestoreBridge::RecordRejectedFailure(
    WorldComponentResourceBindingRestoreStatus status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(status);
}

WorldComponentResourceBindingRestoreResult WorldComponentResourceBindingRestoreBridge::RecordRejectedFailure(
    WorldComponentResourceBindingRestoreStatus status,
    const WorldComponentResourceBindingRestoreState &state) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(
        status,
        WorldComponentResourceBindingStatus::Success,
        ResourceStatus::Success,
        state);
}

WorldComponentResourceBindingRestoreResult WorldComponentResourceBindingRestoreBridge::RecordRejectedFailure(
    WorldComponentResourceBindingRestoreStatus status,
    ResourceStatus resource_status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(status, resource_status);
}

WorldComponentResourceBindingRestoreResult WorldComponentResourceBindingRestoreBridge::RecordRejectedFailure(
    WorldComponentResourceBindingRestoreStatus status,
    ResourceStatus resource_status,
    const WorldComponentResourceBindingRestoreState &state) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(
        status,
        WorldComponentResourceBindingStatus::Success,
        resource_status,
        state);
}

WorldComponentResourceBindingRestoreResult WorldComponentResourceBindingRestoreBridge::RecordSuccess(
    const WorldComponentResourceBindingRestoreState &state) {
    snapshot_.restored_binding_count += state.restored_binding_count;
    snapshot_.last_status = WorldComponentResourceBindingRestoreStatus::Success;
    snapshot_.last_binding_status = WorldComponentResourceBindingStatus::Success;
    snapshot_.last_resource_status = ResourceStatus::Success;
    return WorldComponentResourceBindingRestoreResult::Success(state);
}

WorldComponentResourceBindingRestoreStatus WorldComponentResourceBindingRestoreBridge::ValidateBridgeCapacity() const {
    if (binding_capacity_ == 0U) {
        return WorldComponentResourceBindingRestoreStatus::InvalidBridgeCapacity;
    }

    return WorldComponentResourceBindingRestoreStatus::Success;
}

WorldComponentResourceBindingRestoreStatus WorldComponentResourceBindingRestoreBridge::ValidateDestination(
    const WorldComponentResourceBindingBridge &destination_bridge,
    std::uint32_t input_binding_count) const {
    const WorldComponentResourceBindingStatus destination_status =
        destination_bridge.ValidateRestoreDestination(input_binding_count);
    if (destination_status == WorldComponentResourceBindingStatus::Success) {
        return WorldComponentResourceBindingRestoreStatus::Success;
    }

    if (destination_status == WorldComponentResourceBindingStatus::InvalidBridgeCapacity) {
        return WorldComponentResourceBindingRestoreStatus::InvalidBridgeCapacity;
    }

    if (destination_status == WorldComponentResourceBindingStatus::CapacityExceeded) {
        return WorldComponentResourceBindingRestoreStatus::DestinationCapacityExceeded;
    }

    if (destination_status == WorldComponentResourceBindingStatus::DuplicateComponentBinding) {
        return WorldComponentResourceBindingRestoreStatus::DestinationNotEmpty;
    }

    return WorldComponentResourceBindingRestoreStatus::BindingApplyFailed;
}

WorldComponentResourceBindingRestoreStatus WorldComponentResourceBindingRestoreBridge::ValidateRecords(
    const WorldComponentAttachmentBridge &attachment_source,
    const ResourceRegistry &resource_registry,
    const WorldComponentResourceBinding *input_bindings,
    std::uint32_t input_binding_count,
    ResourceStatus *out_resource_status,
    WorldComponentResourceBindingRestoreState *state) const {
    std::uint32_t record_index = 0U;
    while (record_index < input_binding_count) {
        const WorldComponentResourceBindingRestoreStatus status = ValidateRecord(
            attachment_source,
            resource_registry,
            input_bindings,
            record_index,
            out_resource_status);
        if (status != WorldComponentResourceBindingRestoreStatus::Success) {
            if (state != nullptr) {
                state->rejected_binding_count = record_index + 1U;
            }

            return status;
        }

        ++record_index;
    }

    return WorldComponentResourceBindingRestoreStatus::Success;
}

WorldComponentResourceBindingRestoreStatus WorldComponentResourceBindingRestoreBridge::ValidateRecord(
    const WorldComponentAttachmentBridge &attachment_source,
    const ResourceRegistry &resource_registry,
    const WorldComponentResourceBinding *input_bindings,
    std::uint32_t record_index,
    ResourceStatus *out_resource_status) const {
    const WorldComponentResourceBinding &binding = input_bindings[record_index];
    if (!binding.world_object_id.IsValid()) {
        return WorldComponentResourceBindingRestoreStatus::InvalidWorldObjectId;
    }

    if (!binding.component_type_id.IsValid()) {
        return WorldComponentResourceBindingRestoreStatus::InvalidComponentTypeId;
    }

    if (!binding.component_slot_id.IsValid()) {
        return WorldComponentResourceBindingRestoreStatus::InvalidComponentSlotId;
    }

    if (!binding.resource_handle.IsValid()) {
        if (out_resource_status != nullptr) {
            *out_resource_status = ResourceStatus::InvalidHandle;
        }

        return WorldComponentResourceBindingRestoreStatus::InvalidResourceHandle;
    }

    if (!binding.expected_resource_type.IsValid()) {
        return WorldComponentResourceBindingRestoreStatus::InvalidResourceTypeId;
    }

    if (HasDuplicateInput(input_bindings, record_index)) {
        return WorldComponentResourceBindingRestoreStatus::DuplicateInputBinding;
    }

    if (!HasAttachmentTuple(attachment_source, binding)) {
        return WorldComponentResourceBindingRestoreStatus::MissingAttachment;
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

    return WorldComponentResourceBindingRestoreStatus::Success;
}

bool WorldComponentResourceBindingRestoreBridge::HasAttachmentTuple(
    const WorldComponentAttachmentBridge &attachment_source,
    const WorldComponentResourceBinding &binding) const {
    std::array<WorldComponentAttachment, MAX_WORLD_OBJECT_COUNT> attachments{};
    const std::uint32_t attachment_count =
        attachment_source.ExportAttachments(attachments.data(), MAX_WORLD_OBJECT_COUNT);
    std::uint32_t scan_count = attachment_count;
    if (scan_count > MAX_WORLD_OBJECT_COUNT) {
        scan_count = MAX_WORLD_OBJECT_COUNT;
    }

    std::uint32_t attachment_index = 0U;
    while (attachment_index < scan_count) {
        const WorldComponentAttachment &attachment = attachments[attachment_index];
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

bool WorldComponentResourceBindingRestoreBridge::HasDuplicateInput(
    const WorldComponentResourceBinding *input_bindings,
    std::uint32_t record_index) const {
    const WorldComponentResourceBinding &binding = input_bindings[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldComponentResourceBinding &compare_binding = input_bindings[compare_index];
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

std::uint32_t WorldComponentResourceBindingRestoreBridge::CountProjectedAcquire(
    const WorldComponentResourceBinding *input_bindings,
    std::uint32_t record_index) const {
    const WorldComponentResourceBinding &binding = input_bindings[record_index];
    std::uint32_t projected_acquire_count = 0U;
    std::uint32_t compare_index = 0U;
    while (compare_index <= record_index) {
        const WorldComponentResourceBinding &compare_binding = input_bindings[compare_index];
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

WorldComponentResourceBindingRestoreStatus WorldComponentResourceBindingRestoreBridge::MapBindingStatus(
    WorldComponentResourceBindingStatus binding_status) const {
    if (binding_status == WorldComponentResourceBindingStatus::InvalidBridgeCapacity) {
        return WorldComponentResourceBindingRestoreStatus::InvalidBridgeCapacity;
    }

    if (binding_status == WorldComponentResourceBindingStatus::InvalidAttachmentSource) {
        return WorldComponentResourceBindingRestoreStatus::InvalidAttachmentSource;
    }

    if (binding_status == WorldComponentResourceBindingStatus::InvalidResourceRegistry) {
        return WorldComponentResourceBindingRestoreStatus::InvalidResourceRegistry;
    }

    if (binding_status == WorldComponentResourceBindingStatus::InvalidWorldObjectId) {
        return WorldComponentResourceBindingRestoreStatus::InvalidWorldObjectId;
    }

    if (binding_status == WorldComponentResourceBindingStatus::InvalidComponentTypeId) {
        return WorldComponentResourceBindingRestoreStatus::InvalidComponentTypeId;
    }

    if (binding_status == WorldComponentResourceBindingStatus::InvalidComponentSlotId) {
        return WorldComponentResourceBindingRestoreStatus::InvalidComponentSlotId;
    }

    if (binding_status == WorldComponentResourceBindingStatus::AttachmentNotFound) {
        return WorldComponentResourceBindingRestoreStatus::MissingAttachment;
    }

    if (binding_status == WorldComponentResourceBindingStatus::InvalidResourceHandle) {
        return WorldComponentResourceBindingRestoreStatus::InvalidResourceHandle;
    }

    if (binding_status == WorldComponentResourceBindingStatus::StaleResourceHandle) {
        return WorldComponentResourceBindingRestoreStatus::StaleResourceHandle;
    }

    if (binding_status == WorldComponentResourceBindingStatus::ResourceTypeMismatch) {
        return WorldComponentResourceBindingRestoreStatus::ResourceTypeMismatch;
    }

    if (binding_status == WorldComponentResourceBindingStatus::CapacityExceeded) {
        return WorldComponentResourceBindingRestoreStatus::DestinationCapacityExceeded;
    }

    if (binding_status == WorldComponentResourceBindingStatus::ResourceAcquireFailed) {
        return WorldComponentResourceBindingRestoreStatus::ResourceAcquireFailed;
    }

    return WorldComponentResourceBindingRestoreStatus::BindingApplyFailed;
}

WorldComponentResourceBindingRestoreStatus WorldComponentResourceBindingRestoreBridge::MapResourceStatus(
    ResourceStatus resource_status) const {
    if (resource_status == ResourceStatus::InvalidHandle) {
        return WorldComponentResourceBindingRestoreStatus::InvalidResourceHandle;
    }

    if (resource_status == ResourceStatus::GenerationMismatch) {
        return WorldComponentResourceBindingRestoreStatus::StaleResourceHandle;
    }

    if (resource_status == ResourceStatus::TypeMismatch) {
        return WorldComponentResourceBindingRestoreStatus::ResourceTypeMismatch;
    }

    if (resource_status == ResourceStatus::ReferenceCountOverflow) {
        return WorldComponentResourceBindingRestoreStatus::ResourceAcquireWouldOverflow;
    }

    return WorldComponentResourceBindingRestoreStatus::ResourceAcquireFailed;
}

WorldComponentResourceBindingRestoreResult WorldComponentResourceBindingRestoreBridge::RollbackAndRecordApplyFailure(
    WorldComponentResourceBindingBridge *destination_bridge,
    ResourceRegistry *resource_registry,
    const WorldComponentResourceBindingRestoreState &state,
    const WorldComponentResourceBindingResult &binding_result) {
    const WorldComponentResourceBindingRestoreStatus restore_status = MapBindingStatus(binding_result.status);
    if (state.restored_binding_count == 0U) {
        return RecordFailure(restore_status, binding_result.status, binding_result.resource_status);
    }

    ++snapshot_.rollback_count;
    const WorldComponentResourceBindingStatus clear_status = destination_bridge->Clear(resource_registry);
    if (clear_status != WorldComponentResourceBindingStatus::Success) {
        const WorldComponentResourceBindingSnapshot destination_snapshot = destination_bridge->Snapshot();
        return RecordFailure(
            WorldComponentResourceBindingRestoreStatus::RollbackFailed,
            clear_status,
            destination_snapshot.last_resource_status);
    }

    return RecordFailure(restore_status, binding_result.status, binding_result.resource_status);
}
}
