#include "YuEngine/Object/ObjectRegistry.h"

#include <limits>

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::object {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requestedCapacity, std::uint32_t maximumCapacity) {
    if (requestedCapacity > maximumCapacity) {
        return maximumCapacity;
    }

    return requestedCapacity;
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

    ObjectSlot* freeSlot = nullptr;
    std::uint32_t freeSlotIndex = 0U;
    std::uint32_t slotIndex = 0U;
    for (ObjectSlot& slot : slots_) {
        if (slotIndex >= snapshot_.object_capacity) {
            break;
        }

        if (slot.is_active) {
            ++slotIndex;
            continue;
        }

        freeSlot = &slot;
        freeSlotIndex = slotIndex;
        break;
    }

    if (freeSlot == nullptr) {
        return ObjectRegistrationResult::Failure(RecordFailure(ObjectStatus::CapacityExceeded));
    }

    const ObjectStatus typeStatus = RegisterTypeIfNeeded(descriptor.type);
    if (typeStatus != ObjectStatus::Success) {
        return ObjectRegistrationResult::Failure(RecordFailure(typeStatus));
    }

    if (freeSlot->generation == INVALID_OBJECT_GENERATION) {
        freeSlot->generation = 1U;
    }

    freeSlot->type = descriptor.type;
    freeSlot->reference_count = descriptor.initial_reference_count;
    freeSlot->is_active = true;
    ++snapshot_.alive_object_count;
    ++snapshot_.created_object_count;
    snapshot_.referenced_object_count += descriptor.initial_reference_count;
    RecordSuccess();
    return ObjectRegistrationResult::Success(ObjectHandle{freeSlotIndex, freeSlot->generation});
}

ObjectStatus ObjectRegistry::Validate(ObjectHandle handle) {
    std::size_t slotIndex = 0U;
    const ObjectStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ObjectStatus::Success) {
        return RecordFailure(handleStatus);
    }

    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::Acquire(ObjectHandle handle) {
    std::size_t slotIndex = 0U;
    const ObjectStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ObjectStatus::Success) {
        return RecordFailure(handleStatus);
    }

    ObjectSlot& slot = slots_[slotIndex];
    if (slot.reference_count == std::numeric_limits<std::uint32_t>::max()) {
        return RecordFailure(ObjectStatus::ReferenceCountOverflow);
    }

    ++slot.reference_count;
    ++snapshot_.referenced_object_count;
    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::Release(ObjectHandle handle) {
    std::size_t slotIndex = 0U;
    const ObjectStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ObjectStatus::Success) {
        return RecordFailure(handleStatus);
    }

    ObjectSlot& slot = slots_[slotIndex];
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
    std::size_t slotIndex = 0U;
    const ObjectStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ObjectStatus::Success) {
        return RecordFailure(handleStatus);
    }

    ObjectSlot& slot = slots_[slotIndex];
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

ObjectStatus ObjectRegistry::ResolveHandle(ObjectHandle handle, std::size_t& outIndex) const {
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

    outIndex = handle.slot;
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
    for (const ObjectTypeId& registeredType : types_) {
        if (index >= snapshot_.type_count) {
            return false;
        }

        if (registeredType.value == type.value) {
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
