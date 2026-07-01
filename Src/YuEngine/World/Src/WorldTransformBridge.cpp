// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldTransformBridge.cpp

#include "YuEngine/World/WorldTransformBridge.h"

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

WorldTransformBridge::WorldTransformBridge(WorldInstance &world, WorldTransformBridgeDesc desc)
    : world_(world),
      bindings_{},
      snapshot_{
          ClampCapacity(desc.bridge_capacity, MAX_WORLD_OBJECT_COUNT),
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          WorldObjectId{},
          0U,
          0U,
          WorldTransformState{},
          WorldTransformStatus::Success} {
    if (desc.bridge_capacity == 0U) {
        snapshot_.last_status = WorldTransformStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldTransformResult WorldTransformBridge::Register(WorldObjectId world_object_id,
    const WorldTransformState &transform_state) {
    const WorldTransformStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldTransformStatus::Success) {
        return WorldTransformResult::Failure(RecordFailure(capacity_status));
    }

    if (!world_object_id.IsValid()) {
        return WorldTransformResult::Failure(RecordFailure(WorldTransformStatus::InvalidWorldObjectId));
    }

    if (!world_.ContainsObject(world_object_id)) {
        return WorldTransformResult::Failure(RecordFailure(WorldTransformStatus::MissingWorldObject));
    }

    if (FindBindingByWorldObjectId(world_object_id) != nullptr) {
        return WorldTransformResult::Failure(RecordFailure(WorldTransformStatus::DuplicateWorldObjectId));
    }

    if (snapshot_.record_count >= snapshot_.bridge_capacity) {
        const std::uint32_t transform_slot = snapshot_.record_count;
        const WorldTransformStatus status =
            RecordCapacityFailure(world_object_id, transform_state, transform_slot);
        return WorldTransformResult::Failure(status);
    }

    WorldTransformBinding *binding = FindFreeBinding();
    if (binding == nullptr) {
        const std::uint32_t transform_slot = snapshot_.bridge_capacity;
        const WorldTransformStatus status =
            RecordCapacityFailure(world_object_id, transform_state, transform_slot);
        return WorldTransformResult::Failure(status);
    }

    binding->world_object_id = world_object_id;
    binding->transform_state = transform_state;
    binding->is_bound = true;
    ++snapshot_.record_count;
    RecordSuccess();
    return WorldTransformResult::Success(world_object_id, transform_state);
}

WorldTransformStatus WorldTransformBridge::Set(WorldObjectId world_object_id,
    const WorldTransformState &transform_state) {
    const WorldTransformStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldTransformStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailure(WorldTransformStatus::InvalidWorldObjectId);
    }

    WorldTransformBinding *binding = FindBindingByWorldObjectId(world_object_id);
    if (binding == nullptr) {
        return RecordFailure(WorldTransformStatus::TransformNotFound);
    }

    binding->transform_state = transform_state;
    ++snapshot_.updated_record_count;
    RecordSuccess();
    return WorldTransformStatus::Success;
}

WorldTransformResult WorldTransformBridge::Query(WorldObjectId world_object_id) {
    const WorldTransformStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldTransformStatus::Success) {
        return WorldTransformResult::Failure(RecordFailure(capacity_status));
    }

    if (!world_object_id.IsValid()) {
        return WorldTransformResult::Failure(RecordFailure(WorldTransformStatus::InvalidWorldObjectId));
    }

    const WorldTransformBinding *binding = FindBindingByWorldObjectId(world_object_id);
    if (binding == nullptr) {
        return WorldTransformResult::Failure(RecordFailure(WorldTransformStatus::TransformNotFound));
    }

    RecordSuccess();
    return WorldTransformResult::Success(world_object_id, binding->transform_state);
}

WorldTransformStatus WorldTransformBridge::Remove(WorldObjectId world_object_id) {
    const WorldTransformStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldTransformStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailure(WorldTransformStatus::InvalidWorldObjectId);
    }

    WorldTransformBinding *binding = FindBindingByWorldObjectId(world_object_id);
    if (binding == nullptr) {
        return RecordFailure(WorldTransformStatus::TransformNotFound);
    }

    ClearBinding(*binding);
    ++snapshot_.removed_record_count;
    RecountRecords();
    RecordSuccess();
    return WorldTransformStatus::Success;
}

WorldTransformStatus WorldTransformBridge::Clear() {
    const WorldTransformStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldTransformStatus::Success) {
        return RecordFailure(capacity_status);
    }

    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        WorldTransformBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        ClearBinding(binding);
        ++snapshot_.removed_record_count;
    }

    RecountRecords();
    RecordSuccess();
    return WorldTransformStatus::Success;
}

WorldTransformSnapshot WorldTransformBridge::Snapshot() const {
    return snapshot_;
}

WorldTransformStatus WorldTransformBridge::RecordCapacityFailure(WorldObjectId world_object_id,
    const WorldTransformState &transform_state,
    std::uint32_t transform_slot) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_failed_world_object_id = world_object_id;
    snapshot_.last_failed_transform_slot = transform_slot;
    snapshot_.last_required_transform_capacity = transform_slot + 1U;
    snapshot_.last_failed_transform_state = transform_state;
    snapshot_.last_status = WorldTransformStatus::CapacityExceeded;
    return WorldTransformStatus::CapacityExceeded;
}

WorldTransformStatus WorldTransformBridge::RecordFailure(WorldTransformStatus status) {
    ClearCapacityEntry();
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return status;
}

void WorldTransformBridge::RecordSuccess() {
    ClearCapacityEntry();
    snapshot_.last_status = WorldTransformStatus::Success;
}

void WorldTransformBridge::ClearCapacityEntry() {
    snapshot_.last_failed_world_object_id = WorldObjectId{};
    snapshot_.last_failed_transform_slot = 0U;
    snapshot_.last_required_transform_capacity = 0U;
    snapshot_.last_failed_transform_state = WorldTransformState{};
}

WorldTransformStatus WorldTransformBridge::ValidateBridgeCapacity() const {
    if (snapshot_.bridge_capacity == 0U) {
        return WorldTransformStatus::InvalidBridgeCapacity;
    }

    return WorldTransformStatus::Success;
}

WorldTransformBinding *WorldTransformBridge::FindBindingByWorldObjectId(WorldObjectId world_object_id) {
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        WorldTransformBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        if (binding.world_object_id.value == world_object_id.value) {
            return &binding;
        }
    }

    return nullptr;
}

const WorldTransformBinding *WorldTransformBridge::FindBindingByWorldObjectId(WorldObjectId world_object_id) const {
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        const WorldTransformBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        if (binding.world_object_id.value == world_object_id.value) {
            return &binding;
        }
    }

    return nullptr;
}

WorldTransformBinding *WorldTransformBridge::FindFreeBinding() {
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        WorldTransformBinding &binding = bindings_[index];
        if (binding.is_bound) {
            continue;
        }

        return &binding;
    }

    return nullptr;
}

void WorldTransformBridge::ClearBinding(WorldTransformBinding &binding) {
    binding.world_object_id = WorldObjectId{};
    binding.transform_state = WorldTransformState{};
    binding.is_bound = false;
}

void WorldTransformBridge::RecountRecords() {
    std::uint32_t record_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.bridge_capacity; ++index) {
        const WorldTransformBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        ++record_count;
    }

    snapshot_.record_count = record_count;
}
}
