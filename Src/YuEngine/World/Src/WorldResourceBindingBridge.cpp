// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldResourceBindingBridge.cpp

#include "YuEngine/World/WorldResourceBindingBridge.h"

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceRegistry.h"

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

WorldResourceBindingBridge::WorldResourceBindingBridge(WorldResourceBindingBridgeDesc desc)
    : bindings_{},
      snapshot_{
          ClampCapacity(desc.bridge_capacity, MAX_WORLD_OBJECT_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          ResourceStatus::Success,
          WorldResourceBindingStatus::Success} {
    if (desc.bridge_capacity == 0U) {
        snapshot_.last_status = WorldResourceBindingStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldResourceBindingResult WorldResourceBindingBridge::Bind(
    ResourceRegistry *resource_registry,
    WorldObjectId world_object_id,
    ResourceHandle resource_handle,
    ResourceTypeId expected_resource_type) {
    const WorldResourceBindingStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldResourceBindingStatus::Success) {
        return RecordFailureResult(capacity_status);
    }

    if (resource_registry == nullptr) {
        return RecordFailureResult(WorldResourceBindingStatus::InvalidResourceRegistry);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailureResult(WorldResourceBindingStatus::InvalidWorldObjectId);
    }

    if (FindBindingByWorldObjectId(world_object_id) != nullptr) {
        return RecordFailureResult(WorldResourceBindingStatus::DuplicateWorldObjectId);
    }

    if (snapshot_.active_binding_count >= snapshot_.bridge_capacity) {
        return RecordFailureResult(WorldResourceBindingStatus::CapacityExceeded);
    }

    WorldResourceBinding *binding = FindFreeBinding();
    if (binding == nullptr) {
        return RecordFailureResult(WorldResourceBindingStatus::CapacityExceeded);
    }

    const ResourceStatus acquire_status = resource_registry->Acquire(resource_handle, expected_resource_type);
    if (acquire_status != ResourceStatus::Success) {
        const WorldResourceBindingStatus bridge_status = MapAcquireStatus(acquire_status);
        return RecordFailureResult(bridge_status, acquire_status);
    }

    binding->world_object_id = world_object_id;
    binding->resource_handle = resource_handle;
    binding->expected_resource_type = expected_resource_type;
    binding->is_bound = true;
    binding->is_acquired = true;
    ++snapshot_.active_binding_count;
    ++snapshot_.acquired_binding_count;
    RecordSuccess(acquire_status);
    return WorldResourceBindingResult::Success(world_object_id, resource_handle, expected_resource_type);
}

WorldResourceBindingResult WorldResourceBindingBridge::Query(WorldObjectId world_object_id) {
    const WorldResourceBindingStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldResourceBindingStatus::Success) {
        return RecordFailureResult(capacity_status);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailureResult(WorldResourceBindingStatus::InvalidWorldObjectId);
    }

    const WorldResourceBinding *binding = FindBindingByWorldObjectId(world_object_id);
    if (binding == nullptr) {
        return RecordFailureResult(WorldResourceBindingStatus::BindingNotFound);
    }

    RecordSuccess(ResourceStatus::Success);
    return WorldResourceBindingResult::Success(
        binding->world_object_id,
        binding->resource_handle,
        binding->expected_resource_type);
}

WorldResourceBindingStatus WorldResourceBindingBridge::Remove(
    ResourceRegistry *resource_registry,
    WorldObjectId world_object_id) {
    const WorldResourceBindingStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldResourceBindingStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (resource_registry == nullptr) {
        return RecordFailure(WorldResourceBindingStatus::InvalidResourceRegistry);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailure(WorldResourceBindingStatus::InvalidWorldObjectId);
    }

    WorldResourceBinding *binding = FindBindingByWorldObjectId(world_object_id);
    if (binding == nullptr) {
        return RecordFailure(WorldResourceBindingStatus::BindingNotFound);
    }

    const ResourceStatus release_status = resource_registry->Release(binding->resource_handle);
    if (release_status != ResourceStatus::Success) {
        return RecordFailure(WorldResourceBindingStatus::ResourceReleaseFailed, release_status);
    }

    ClearBinding(*binding);
    ++snapshot_.released_binding_count;
    RecountActiveBindings();
    RecordSuccess(release_status);
    return WorldResourceBindingStatus::Success;
}

WorldResourceBindingStatus WorldResourceBindingBridge::Clear(ResourceRegistry *resource_registry) {
    const WorldResourceBindingStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldResourceBindingStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (resource_registry == nullptr) {
        return RecordFailure(WorldResourceBindingStatus::InvalidResourceRegistry);
    }

    WorldResourceBindingStatus result = WorldResourceBindingStatus::Success;
    ResourceStatus last_resource_status = ResourceStatus::Success;
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        WorldResourceBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        const ResourceStatus release_status = resource_registry->Release(binding.resource_handle);
        if (release_status != ResourceStatus::Success) {
            result = WorldResourceBindingStatus::ResourceReleaseFailed;
            last_resource_status = release_status;
            break;
        }

        ClearBinding(binding);
        ++snapshot_.released_binding_count;
        ++snapshot_.cleared_binding_count;
    }

    RecountActiveBindings();
    if (result != WorldResourceBindingStatus::Success) {
        return RecordFailure(result, last_resource_status);
    }

    RecordSuccess(last_resource_status);
    return WorldResourceBindingStatus::Success;
}

WorldResourceBindingSnapshot WorldResourceBindingBridge::Snapshot() const {
    return snapshot_;
}

WorldResourceBindingResult WorldResourceBindingBridge::RecordFailureResult(WorldResourceBindingStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_resource_status = ResourceStatus::Success;
    return WorldResourceBindingResult::Failure(status);
}

WorldResourceBindingResult WorldResourceBindingBridge::RecordFailureResult(
    WorldResourceBindingStatus status,
    ResourceStatus resource_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_resource_status = resource_status;
    return WorldResourceBindingResult::Failure(status, resource_status);
}

WorldResourceBindingStatus WorldResourceBindingBridge::RecordFailure(WorldResourceBindingStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_resource_status = ResourceStatus::Success;
    return status;
}

WorldResourceBindingStatus WorldResourceBindingBridge::RecordFailure(
    WorldResourceBindingStatus status,
    ResourceStatus resource_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_resource_status = resource_status;
    return status;
}

void WorldResourceBindingBridge::RecordSuccess(ResourceStatus resource_status) {
    snapshot_.last_status = WorldResourceBindingStatus::Success;
    snapshot_.last_resource_status = resource_status;
}

WorldResourceBindingStatus WorldResourceBindingBridge::ValidateBridgeCapacity() const {
    if (snapshot_.bridge_capacity == 0U) {
        return WorldResourceBindingStatus::InvalidBridgeCapacity;
    }

    return WorldResourceBindingStatus::Success;
}

WorldResourceBindingStatus WorldResourceBindingBridge::MapAcquireStatus(ResourceStatus resource_status) const {
    if (resource_status == ResourceStatus::InvalidHandle) {
        return WorldResourceBindingStatus::InvalidResourceHandle;
    }

    if (resource_status == ResourceStatus::GenerationMismatch) {
        return WorldResourceBindingStatus::StaleResourceHandle;
    }

    if (resource_status == ResourceStatus::TypeMismatch) {
        return WorldResourceBindingStatus::ResourceTypeMismatch;
    }

    return WorldResourceBindingStatus::ResourceAcquireFailed;
}

WorldResourceBinding *WorldResourceBindingBridge::FindBindingByWorldObjectId(WorldObjectId world_object_id) {
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        WorldResourceBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        if (binding.world_object_id.value == world_object_id.value) {
            return &binding;
        }
    }

    return nullptr;
}

const WorldResourceBinding *WorldResourceBindingBridge::FindBindingByWorldObjectId(WorldObjectId world_object_id) const {
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        const WorldResourceBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        if (binding.world_object_id.value == world_object_id.value) {
            return &binding;
        }
    }

    return nullptr;
}

WorldResourceBinding *WorldResourceBindingBridge::FindFreeBinding() {
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        WorldResourceBinding &binding = bindings_[index];
        if (binding.is_bound) {
            continue;
        }

        return &binding;
    }

    return nullptr;
}

void WorldResourceBindingBridge::ClearBinding(WorldResourceBinding &binding) {
    binding.world_object_id = WorldObjectId{};
    binding.resource_handle = ResourceHandle{};
    binding.expected_resource_type = ResourceTypeId{};
    binding.is_bound = false;
    binding.is_acquired = false;
}

void WorldResourceBindingBridge::RecountActiveBindings() {
    std::uint32_t active_binding_count = 0U;
    std::uint32_t acquired_binding_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        const WorldResourceBinding &binding = bindings_[index];
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
