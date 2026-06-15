// Module: YuEngine World
// File: Src/YuEngine/World/Src/WorldComponentResourceBindingBridge.cpp

#include "YuEngine/World/WorldComponentResourceBindingBridge.h"

#include <array>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/World/WorldComponentAttachmentBridge.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceStatus;
using yuengine::resource::ResourceTypeId;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

WorldComponentResourceBindingBridge::WorldComponentResourceBindingBridge(
    WorldComponentResourceBindingBridgeDesc desc)
    : bindings_{},
      snapshot_{
          ClampCapacity(desc.binding_capacity, MAX_WORLD_OBJECT_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          ResourceStatus::Success,
          WorldComponentResourceBindingStatus::Success} {
    if (desc.binding_capacity == 0U) {
        snapshot_.last_status = WorldComponentResourceBindingStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldComponentResourceBindingResult WorldComponentResourceBindingBridge::Bind(
    const WorldComponentAttachmentBridge *attachment_source,
    ResourceRegistry *resource_registry,
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id,
    ResourceHandle resource_handle,
    ResourceTypeId expected_resource_type) {
    const WorldComponentResourceBindingStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentResourceBindingStatus::Success) {
        return RecordFailureResult(capacity_status);
    }

    if (attachment_source == nullptr) {
        return RecordFailureResult(WorldComponentResourceBindingStatus::InvalidAttachmentSource);
    }

    if (resource_registry == nullptr) {
        return RecordFailureResult(WorldComponentResourceBindingStatus::InvalidResourceRegistry);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailureResult(WorldComponentResourceBindingStatus::InvalidWorldObjectId);
    }

    if (!component_type_id.IsValid()) {
        return RecordFailureResult(WorldComponentResourceBindingStatus::InvalidComponentTypeId);
    }

    if (!component_slot_id.IsValid()) {
        return RecordFailureResult(WorldComponentResourceBindingStatus::InvalidComponentSlotId);
    }

    const WorldComponentResourceBindingStatus attachment_status =
        ValidateAttachmentTuple(attachment_source, world_object_id, component_type_id, component_slot_id);
    if (attachment_status != WorldComponentResourceBindingStatus::Success) {
        return RecordFailureResult(attachment_status);
    }

    if (FindBinding(world_object_id, component_type_id, component_slot_id) != nullptr) {
        return RecordFailureResult(WorldComponentResourceBindingStatus::DuplicateComponentBinding);
    }

    if (snapshot_.active_binding_count >= snapshot_.binding_capacity) {
        return RecordFailureResult(WorldComponentResourceBindingStatus::CapacityExceeded);
    }

    WorldComponentResourceBinding *binding = FindFreeBinding();
    if (binding == nullptr) {
        return RecordFailureResult(WorldComponentResourceBindingStatus::CapacityExceeded);
    }

    const ResourceStatus acquire_status = resource_registry->Acquire(resource_handle, expected_resource_type);
    if (acquire_status != ResourceStatus::Success) {
        const WorldComponentResourceBindingStatus bridge_status = MapAcquireStatus(acquire_status);
        return RecordFailureResult(bridge_status, acquire_status);
    }

    binding->world_object_id = world_object_id;
    binding->component_type_id = component_type_id;
    binding->component_slot_id = component_slot_id;
    binding->resource_handle = resource_handle;
    binding->expected_resource_type = expected_resource_type;
    binding->is_bound = true;
    binding->is_acquired = true;
    ++snapshot_.active_binding_count;
    ++snapshot_.acquired_binding_count;
    RecordSuccess(acquire_status);
    return WorldComponentResourceBindingResult::Success(
        world_object_id,
        component_type_id,
        component_slot_id,
        resource_handle,
        expected_resource_type);
}

WorldComponentResourceBindingResult WorldComponentResourceBindingBridge::Query(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id) {
    const WorldComponentResourceBindingStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentResourceBindingStatus::Success) {
        return RecordFailureResult(capacity_status);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailureResult(WorldComponentResourceBindingStatus::InvalidWorldObjectId);
    }

    if (!component_type_id.IsValid()) {
        return RecordFailureResult(WorldComponentResourceBindingStatus::InvalidComponentTypeId);
    }

    if (!component_slot_id.IsValid()) {
        return RecordFailureResult(WorldComponentResourceBindingStatus::InvalidComponentSlotId);
    }

    ++snapshot_.query_count;
    const WorldComponentResourceBinding *binding = FindBinding(world_object_id, component_type_id, component_slot_id);
    if (binding == nullptr) {
        return RecordFailureResult(WorldComponentResourceBindingStatus::BindingNotFound);
    }

    RecordSuccess(ResourceStatus::Success);
    return WorldComponentResourceBindingResult::Success(
        binding->world_object_id,
        binding->component_type_id,
        binding->component_slot_id,
        binding->resource_handle,
        binding->expected_resource_type);
}

WorldComponentResourceBindingStatus WorldComponentResourceBindingBridge::Remove(
    ResourceRegistry *resource_registry,
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id) {
    const WorldComponentResourceBindingStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentResourceBindingStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (resource_registry == nullptr) {
        return RecordFailure(WorldComponentResourceBindingStatus::InvalidResourceRegistry);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailure(WorldComponentResourceBindingStatus::InvalidWorldObjectId);
    }

    if (!component_type_id.IsValid()) {
        return RecordFailure(WorldComponentResourceBindingStatus::InvalidComponentTypeId);
    }

    if (!component_slot_id.IsValid()) {
        return RecordFailure(WorldComponentResourceBindingStatus::InvalidComponentSlotId);
    }

    WorldComponentResourceBinding *binding = FindBinding(world_object_id, component_type_id, component_slot_id);
    if (binding == nullptr) {
        return RecordFailure(WorldComponentResourceBindingStatus::BindingNotFound);
    }

    const ResourceStatus release_status = resource_registry->Release(binding->resource_handle);
    if (release_status != ResourceStatus::Success) {
        return RecordFailure(WorldComponentResourceBindingStatus::ResourceReleaseFailed, release_status);
    }

    ClearBinding(*binding);
    ++snapshot_.released_binding_count;
    RecountActiveBindings();
    RecordSuccess(release_status);
    return WorldComponentResourceBindingStatus::Success;
}

WorldComponentResourceBindingStatus WorldComponentResourceBindingBridge::Clear(ResourceRegistry *resource_registry) {
    const WorldComponentResourceBindingStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentResourceBindingStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (resource_registry == nullptr) {
        return RecordFailure(WorldComponentResourceBindingStatus::InvalidResourceRegistry);
    }

    WorldComponentResourceBindingStatus result = WorldComponentResourceBindingStatus::Success;
    ResourceStatus last_resource_status = ResourceStatus::Success;
    for (std::uint32_t index = 0U; index < snapshot_.binding_capacity; ++index) {
        WorldComponentResourceBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        const ResourceStatus release_status = resource_registry->Release(binding.resource_handle);
        if (release_status != ResourceStatus::Success) {
            result = WorldComponentResourceBindingStatus::ResourceReleaseFailed;
            last_resource_status = release_status;
            break;
        }

        ClearBinding(binding);
        ++snapshot_.released_binding_count;
        ++snapshot_.cleared_binding_count;
    }

    RecountActiveBindings();
    if (result != WorldComponentResourceBindingStatus::Success) {
        return RecordFailure(result, last_resource_status);
    }

    RecordSuccess(last_resource_status);
    return WorldComponentResourceBindingStatus::Success;
}

WorldComponentResourceBindingSnapshot WorldComponentResourceBindingBridge::Snapshot() const {
    return snapshot_;
}

std::uint32_t WorldComponentResourceBindingBridge::ExportBindings(
    WorldComponentResourceBinding *output_bindings,
    std::uint32_t output_capacity) const {
    std::uint32_t exported_binding_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.binding_capacity; ++index) {
        const WorldComponentResourceBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        if ((output_bindings != nullptr) && (exported_binding_count < output_capacity)) {
            output_bindings[exported_binding_count] = binding;
        }

        ++exported_binding_count;
    }

    return exported_binding_count;
}

WorldComponentResourceBindingResult WorldComponentResourceBindingBridge::RecordFailureResult(
    WorldComponentResourceBindingStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_resource_status = ResourceStatus::Success;
    return WorldComponentResourceBindingResult::Failure(status);
}

WorldComponentResourceBindingResult WorldComponentResourceBindingBridge::RecordFailureResult(
    WorldComponentResourceBindingStatus status,
    ResourceStatus resource_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_resource_status = resource_status;
    return WorldComponentResourceBindingResult::Failure(status, resource_status);
}

WorldComponentResourceBindingStatus WorldComponentResourceBindingBridge::RecordFailure(
    WorldComponentResourceBindingStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_resource_status = ResourceStatus::Success;
    return status;
}

WorldComponentResourceBindingStatus WorldComponentResourceBindingBridge::RecordFailure(
    WorldComponentResourceBindingStatus status,
    ResourceStatus resource_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_resource_status = resource_status;
    return status;
}

void WorldComponentResourceBindingBridge::RecordSuccess(ResourceStatus resource_status) {
    snapshot_.last_status = WorldComponentResourceBindingStatus::Success;
    snapshot_.last_resource_status = resource_status;
}

WorldComponentResourceBindingStatus WorldComponentResourceBindingBridge::ValidateBridgeCapacity() const {
    if (snapshot_.binding_capacity == 0U) {
        return WorldComponentResourceBindingStatus::InvalidBridgeCapacity;
    }

    return WorldComponentResourceBindingStatus::Success;
}

WorldComponentResourceBindingStatus WorldComponentResourceBindingBridge::ValidateAttachmentTuple(
    const WorldComponentAttachmentBridge *attachment_source,
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id) const {
    std::array<WorldComponentAttachment, MAX_WORLD_OBJECT_COUNT> attachments{};
    const std::uint32_t attachment_count =
        attachment_source->ExportAttachments(attachments.data(), MAX_WORLD_OBJECT_COUNT);
    std::uint32_t scan_count = attachment_count;
    if (scan_count > MAX_WORLD_OBJECT_COUNT) {
        scan_count = MAX_WORLD_OBJECT_COUNT;
    }

    for (std::uint32_t index = 0U; index < scan_count; ++index) {
        const WorldComponentAttachment &attachment = attachments[index];
        if (attachment.world_object_id.value != world_object_id.value) {
            continue;
        }

        if (attachment.component_type_id.value != component_type_id.value) {
            continue;
        }

        if (attachment.component_slot_id.value == component_slot_id.value) {
            return WorldComponentResourceBindingStatus::Success;
        }
    }

    return WorldComponentResourceBindingStatus::AttachmentNotFound;
}

WorldComponentResourceBindingStatus WorldComponentResourceBindingBridge::MapAcquireStatus(
    ResourceStatus resource_status) const {
    if (resource_status == ResourceStatus::InvalidHandle) {
        return WorldComponentResourceBindingStatus::InvalidResourceHandle;
    }

    if (resource_status == ResourceStatus::GenerationMismatch) {
        return WorldComponentResourceBindingStatus::StaleResourceHandle;
    }

    if (resource_status == ResourceStatus::TypeMismatch) {
        return WorldComponentResourceBindingStatus::ResourceTypeMismatch;
    }

    return WorldComponentResourceBindingStatus::ResourceAcquireFailed;
}

WorldComponentResourceBinding *WorldComponentResourceBindingBridge::FindBinding(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id) {
    for (std::uint32_t index = 0U; index < snapshot_.binding_capacity; ++index) {
        WorldComponentResourceBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        if (binding.world_object_id.value != world_object_id.value) {
            continue;
        }

        if (binding.component_type_id.value != component_type_id.value) {
            continue;
        }

        if (binding.component_slot_id.value == component_slot_id.value) {
            return &binding;
        }
    }

    return nullptr;
}

const WorldComponentResourceBinding *WorldComponentResourceBindingBridge::FindBinding(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id) const {
    for (std::uint32_t index = 0U; index < snapshot_.binding_capacity; ++index) {
        const WorldComponentResourceBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        if (binding.world_object_id.value != world_object_id.value) {
            continue;
        }

        if (binding.component_type_id.value != component_type_id.value) {
            continue;
        }

        if (binding.component_slot_id.value == component_slot_id.value) {
            return &binding;
        }
    }

    return nullptr;
}

WorldComponentResourceBinding *WorldComponentResourceBindingBridge::FindFreeBinding() {
    for (std::uint32_t index = 0U; index < snapshot_.binding_capacity; ++index) {
        WorldComponentResourceBinding &binding = bindings_[index];
        if (binding.is_bound) {
            continue;
        }

        return &binding;
    }

    return nullptr;
}

void WorldComponentResourceBindingBridge::ClearBinding(WorldComponentResourceBinding &binding) {
    binding.world_object_id = WorldObjectId{};
    binding.component_type_id = WorldComponentTypeId{};
    binding.component_slot_id = WorldComponentSlotId{};
    binding.resource_handle = ResourceHandle{};
    binding.expected_resource_type = ResourceTypeId{};
    binding.is_bound = false;
    binding.is_acquired = false;
}

void WorldComponentResourceBindingBridge::RecountActiveBindings() {
    std::uint32_t active_binding_count = 0U;
    std::uint32_t acquired_binding_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.binding_capacity; ++index) {
        const WorldComponentResourceBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        ++active_binding_count;
        if (binding.is_acquired) {
            ++acquired_binding_count;
        }
    }

    snapshot_.active_binding_count = active_binding_count;
    snapshot_.acquired_binding_count = acquired_binding_count;
}
}
