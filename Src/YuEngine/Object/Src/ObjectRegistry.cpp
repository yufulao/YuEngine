#include "YuEngine/Object/ObjectRegistry.h"

#include <limits>

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::object {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

ObjectRegistry::ObjectRegistry()
    : ObjectRegistry(ObjectRegistryDesc{}) {
}

ObjectRegistry::ObjectRegistry(ObjectRegistryDesc desc)
    : slots_{},
      types_{},
      snapshot_{
          ClampCapacity(desc.object_capacity, MAX_OBJECT_COUNT),
          ClampCapacity(desc.type_capacity, MAX_OBJECT_TYPE_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          ObjectStatus::Success} {
}

ObjectRegistrationResult ObjectRegistry::CreateSyntheticObject(const ObjectDescriptor& descriptor) {
    if (!descriptor.type.IsValid()) {
        return ObjectRegistrationResult::Failure(RecordFailure(ObjectStatus::InvalidType));
    }

    if (snapshot_.alive_object_count >= snapshot_.object_capacity) {
        return ObjectRegistrationResult::Failure(RecordFailure(ObjectStatus::CapacityExceeded));
    }

    ObjectSlot* free_slot = nullptr;
    std::uint32_t free_slot_index = 0U;
    std::uint32_t slot_index = 0U;
    for (ObjectSlot& slot : slots_) {
        if (slot_index >= snapshot_.object_capacity) {
            break;
        }

        if (slot.is_active) {
            ++slot_index;
            continue;
        }

        free_slot = &slot;
        free_slot_index = slot_index;
        break;
    }

    if (free_slot == nullptr) {
        return ObjectRegistrationResult::Failure(RecordFailure(ObjectStatus::CapacityExceeded));
    }

    const ObjectStatus type_status = RegisterTypeIfNeeded(descriptor.type);
    if (type_status != ObjectStatus::Success) {
        return ObjectRegistrationResult::Failure(RecordFailure(type_status));
    }

    if (free_slot->generation == INVALID_OBJECT_GENERATION) {
        free_slot->generation = 1U;
    }

    free_slot->type = descriptor.type;
    free_slot->reference_count = descriptor.initial_reference_count;
    free_slot->is_active = true;
    ++snapshot_.alive_object_count;
    ++snapshot_.created_object_count;
    snapshot_.referenced_object_count += descriptor.initial_reference_count;
    RecordSuccess();
    return ObjectRegistrationResult::Success(ObjectHandle{free_slot_index, free_slot->generation});
}

ObjectStatus ObjectRegistry::Validate(ObjectHandle handle) {
    std::size_t slot_index = 0U;
    const ObjectStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != ObjectStatus::Success) {
        return RecordFailure(handle_status);
    }

    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::Acquire(ObjectHandle handle) {
    std::size_t slot_index = 0U;
    const ObjectStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != ObjectStatus::Success) {
        return RecordFailure(handle_status);
    }

    ObjectSlot& slot = slots_[slot_index];
    if (slot.reference_count == std::numeric_limits<std::uint32_t>::max()) {
        return RecordFailure(ObjectStatus::ReferenceCountOverflow);
    }

    ++slot.reference_count;
    ++snapshot_.referenced_object_count;
    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::Release(ObjectHandle handle) {
    std::size_t slot_index = 0U;
    const ObjectStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != ObjectStatus::Success) {
        return RecordFailure(handle_status);
    }

    ObjectSlot& slot = slots_[slot_index];
    if (slot.reference_count == 0U) {
        return RecordFailure(ObjectStatus::NotAcquired);
    }

    --slot.reference_count;
    --snapshot_.referenced_object_count;
    ++snapshot_.released_reference_count;
    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::Destroy(ObjectHandle handle) {
    std::size_t slot_index = 0U;
    const ObjectStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != ObjectStatus::Success) {
        return RecordFailure(handle_status);
    }

    ObjectSlot& slot = slots_[slot_index];
    if (slot.reference_count != 0U) {
        return RecordFailure(ObjectStatus::StillReferenced);
    }

    slot.is_active = false;
    slot.type = ObjectTypeId{};
    slot.reference_count = 0U;
    AdvanceGeneration(slot);
    --snapshot_.alive_object_count;
    ++snapshot_.destroyed_object_count;
    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectSnapshot ObjectRegistry::Snapshot() const {
    return snapshot_;
}

ObjectStatus ObjectRegistry::RecordFailure(ObjectStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return status;
}

void ObjectRegistry::RecordSuccess() {
    ++snapshot_.accepted_operation_count;
    snapshot_.last_status = ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::ResolveHandle(ObjectHandle handle, std::size_t& out_index) const {
    if (!handle.IsValid()) {
        return ObjectStatus::InvalidHandle;
    }

    if (handle.slot >= snapshot_.object_capacity) {
        return ObjectStatus::InvalidHandle;
    }

    const ObjectSlot& slot = slots_[handle.slot];
    if (slot.generation == INVALID_OBJECT_GENERATION) {
        return ObjectStatus::InvalidHandle;
    }

    if (slot.generation != handle.generation) {
        return ObjectStatus::GenerationMismatch;
    }

    if (!slot.is_active) {
        return ObjectStatus::InvalidHandle;
    }

    out_index = handle.slot;
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::RegisterTypeIfNeeded(ObjectTypeId type) {
    if (HasType(type)) {
        return ObjectStatus::Success;
    }

    if (snapshot_.type_count >= snapshot_.type_capacity) {
        return ObjectStatus::CapacityExceeded;
    }

    types_[snapshot_.type_count] = type;
    ++snapshot_.type_count;
    return ObjectStatus::Success;
}

bool ObjectRegistry::HasType(ObjectTypeId type) const {
    std::uint32_t index = 0U;
    for (const ObjectTypeId& registered_type : types_) {
        if (index >= snapshot_.type_count) {
            return false;
        }

        if (registered_type.value == type.value) {
            return true;
        }

        ++index;
    }

    return false;
}

void ObjectRegistry::AdvanceGeneration(ObjectSlot& slot) {
    if (slot.generation == std::numeric_limits<std::uint32_t>::max()) {
        slot.generation = 1U;
        return;
    }

    ++slot.generation;
}
}
