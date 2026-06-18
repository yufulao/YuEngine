// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldObjectIdentityBridge.cpp

#include "YuEngine/World/WorldObjectIdentityBridge.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::object::ObjectHandle;
using yuengine::object::ObjectStatus;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

WorldObjectIdentityBridge::WorldObjectIdentityBridge(WorldInstance &world,
    yuengine::object::ObjectRegistry &object_registry,
    WorldObjectIdentityBridgeDesc desc)
    : world_(world),
      object_registry_(object_registry),
      bindings_{},
      snapshot_{
          ClampCapacity(desc.bridge_capacity, MAX_WORLD_OBJECT_COUNT),
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          ObjectStatus::Success,
          WorldObjectIdentityStatus::Success} {
    if (desc.bridge_capacity == 0U) {
        snapshot_.last_status = WorldObjectIdentityStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldObjectIdentityResult WorldObjectIdentityBridge::Bind(WorldObjectId world_object_id,
    ObjectHandle object_handle) {
    const WorldObjectIdentityStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldObjectIdentityStatus::Success) {
        return WorldObjectIdentityResult::Failure(RecordFailure(capacity_status));
    }

    if (!world_object_id.IsValid()) {
        return WorldObjectIdentityResult::Failure(RecordFailure(WorldObjectIdentityStatus::InvalidWorldObjectId));
    }

    if (!world_.ContainsObject(world_object_id)) {
        return WorldObjectIdentityResult::Failure(RecordFailure(WorldObjectIdentityStatus::MissingWorldObject));
    }

    if (FindBindingByWorldObjectId(world_object_id) != nullptr) {
        return WorldObjectIdentityResult::Failure(RecordFailure(WorldObjectIdentityStatus::DuplicateWorldObjectId));
    }

    if (FindBindingByObjectHandle(object_handle) != nullptr) {
        return WorldObjectIdentityResult::Failure(RecordFailure(WorldObjectIdentityStatus::DuplicateObjectHandle));
    }

    if (snapshot_.binding_count >= snapshot_.bridge_capacity) {
        return WorldObjectIdentityResult::Failure(RecordFailure(WorldObjectIdentityStatus::CapacityExceeded));
    }

    WorldObjectIdentityBinding *binding = FindFreeBinding();
    if (binding == nullptr) {
        return WorldObjectIdentityResult::Failure(RecordFailure(WorldObjectIdentityStatus::CapacityExceeded));
    }

    const ObjectStatus acquire_status = object_registry_.Acquire(object_handle);
    if (acquire_status != ObjectStatus::Success) {
        const WorldObjectIdentityStatus bridge_status = MapObjectStatus(acquire_status);
        return WorldObjectIdentityResult::Failure(RecordFailure(bridge_status, acquire_status));
    }

    binding->world_object_id = world_object_id;
    binding->object_handle = object_handle;
    binding->is_bound = true;
    binding->is_acquired = true;
    ++snapshot_.binding_count;
    ++snapshot_.acquired_handle_count;
    RecordSuccess(acquire_status);
    return WorldObjectIdentityResult::Success(world_object_id, object_handle);
}

WorldObjectIdentityStatus WorldObjectIdentityBridge::Validate(WorldObjectId world_object_id) {
    const WorldObjectIdentityStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldObjectIdentityStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailure(WorldObjectIdentityStatus::InvalidWorldObjectId);
    }

    const WorldObjectIdentityBinding *binding = FindBindingByWorldObjectId(world_object_id);
    if (binding == nullptr) {
        return RecordFailure(WorldObjectIdentityStatus::BindingNotFound);
    }

    const ObjectStatus object_status = object_registry_.Validate(binding->object_handle);
    if (object_status != ObjectStatus::Success) {
        const WorldObjectIdentityStatus bridge_status = MapObjectStatus(object_status);
        return RecordFailure(bridge_status, object_status);
    }

    RecordSuccess(object_status);
    return WorldObjectIdentityStatus::Success;
}

WorldObjectIdentityStatus WorldObjectIdentityBridge::Remove(WorldObjectId world_object_id) {
    const WorldObjectIdentityStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldObjectIdentityStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailure(WorldObjectIdentityStatus::InvalidWorldObjectId);
    }

    WorldObjectIdentityBinding *binding = FindBindingByWorldObjectId(world_object_id);
    if (binding == nullptr) {
        return RecordFailure(WorldObjectIdentityStatus::BindingNotFound);
    }

    const ObjectStatus release_status = object_registry_.Release(binding->object_handle);
    if (release_status != ObjectStatus::Success) {
        return RecordFailure(WorldObjectIdentityStatus::ObjectReleaseFailed, release_status);
    }

    ClearBinding(*binding);
    ++snapshot_.released_handle_count;
    RecountActiveBindings();
    RecordSuccess(release_status);
    return WorldObjectIdentityStatus::Success;
}

WorldObjectIdentityStatus WorldObjectIdentityBridge::Clear() {
    const WorldObjectIdentityStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldObjectIdentityStatus::Success) {
        return RecordFailure(capacity_status);
    }

    WorldObjectIdentityStatus result = WorldObjectIdentityStatus::Success;
    ObjectStatus last_object_status = ObjectStatus::Success;
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        WorldObjectIdentityBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        const ObjectStatus release_status = object_registry_.Release(binding.object_handle);
        if (release_status != ObjectStatus::Success) {
            result = WorldObjectIdentityStatus::ObjectReleaseFailed;
            last_object_status = release_status;
            continue;
        }

        ClearBinding(binding);
        ++snapshot_.released_handle_count;
    }

    RecountActiveBindings();
    if (result != WorldObjectIdentityStatus::Success) {
        return RecordFailure(result, last_object_status);
    }

    RecordSuccess(last_object_status);
    return WorldObjectIdentityStatus::Success;
}

WorldObjectIdentitySnapshot WorldObjectIdentityBridge::Snapshot() const {
    return snapshot_;
}

WorldObjectIdentityStatus WorldObjectIdentityBridge::RecordFailure(WorldObjectIdentityStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_object_status = ObjectStatus::Success;
    return status;
}

WorldObjectIdentityStatus WorldObjectIdentityBridge::RecordFailure(WorldObjectIdentityStatus status,
    ObjectStatus object_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_object_status = object_status;
    return status;
}

void WorldObjectIdentityBridge::RecordSuccess(ObjectStatus object_status) {
    snapshot_.last_status = WorldObjectIdentityStatus::Success;
    snapshot_.last_object_status = object_status;
}

WorldObjectIdentityStatus WorldObjectIdentityBridge::ValidateBridgeCapacity() const {
    if (snapshot_.bridge_capacity == 0U) {
        return WorldObjectIdentityStatus::InvalidBridgeCapacity;
    }

    return WorldObjectIdentityStatus::Success;
}

WorldObjectIdentityStatus WorldObjectIdentityBridge::MapObjectStatus(ObjectStatus object_status) const {
    if (object_status == ObjectStatus::InvalidHandle) {
        return WorldObjectIdentityStatus::InvalidObjectHandle;
    }

    if (object_status == ObjectStatus::GenerationMismatch) {
        return WorldObjectIdentityStatus::StaleObjectHandle;
    }

    return WorldObjectIdentityStatus::ObjectAcquireFailed;
}

WorldObjectIdentityBinding *WorldObjectIdentityBridge::FindBindingByWorldObjectId(WorldObjectId world_object_id) {
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        WorldObjectIdentityBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        if (binding.world_object_id.value == world_object_id.value) {
            return &binding;
        }
    }

    return nullptr;
}

const WorldObjectIdentityBinding *WorldObjectIdentityBridge::FindBindingByWorldObjectId(WorldObjectId world_object_id) const {
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        const WorldObjectIdentityBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        if (binding.world_object_id.value == world_object_id.value) {
            return &binding;
        }
    }

    return nullptr;
}

WorldObjectIdentityBinding *WorldObjectIdentityBridge::FindBindingByObjectHandle(ObjectHandle object_handle) {
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        WorldObjectIdentityBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        if (binding.object_handle.slot != object_handle.slot) {
            continue;
        }

        if (binding.object_handle.generation == object_handle.generation) {
            return &binding;
        }
    }

    return nullptr;
}

WorldObjectIdentityBinding *WorldObjectIdentityBridge::FindFreeBinding() {
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        WorldObjectIdentityBinding &binding = bindings_[index];
        if (binding.is_bound) {
            continue;
        }

        return &binding;
    }

    return nullptr;
}

void WorldObjectIdentityBridge::ClearBinding(WorldObjectIdentityBinding &binding) {
    binding.world_object_id = WorldObjectId{};
    binding.object_handle = ObjectHandle{};
    binding.is_bound = false;
    binding.is_acquired = false;
}

void WorldObjectIdentityBridge::RecountActiveBindings() {
    std::uint32_t binding_count = 0U;
    std::uint32_t acquired_handle_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        const WorldObjectIdentityBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        ++binding_count;
        if (binding.is_acquired) {
            ++acquired_handle_count;
        }
    }

    snapshot_.binding_count = binding_count;
    snapshot_.acquired_handle_count = acquired_handle_count;
}
}
