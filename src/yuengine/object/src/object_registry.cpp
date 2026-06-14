#include "yuengine/object/object_registry.h"

#include <limits>

using MemoryAccountingStatus = yuengine::memory::MemoryAccountingStatus;

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
    : ObjectRegistry(object_registry_desc_t{}) {
}

ObjectRegistry::ObjectRegistry(object_registry_desc_t desc)
    : _slots{},
      _types{},
      _snapshot{
          ClampCapacity(desc.ObjectCapacity, MAX_OBJECT_COUNT),
          ClampCapacity(desc.TypeCapacity, MAX_OBJECT_TYPE_COUNT),
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

object_registration_result_t ObjectRegistry::CreateSyntheticObject(const object_descriptor_t& descriptor) {
    if (!descriptor.Type.IsValid()) {
        return object_registration_result_t::Failure(RecordFailure(ObjectStatus::InvalidType));
    }

    if (_snapshot.AliveObjectCount >= _snapshot.ObjectCapacity) {
        return object_registration_result_t::Failure(RecordFailure(ObjectStatus::CapacityExceeded));
    }

    object_slot_t* freeSlot = nullptr;
    std::uint32_t freeSlotIndex = 0U;
    std::uint32_t slotIndex = 0U;
    for (object_slot_t& slot : _slots) {
        if (slotIndex >= _snapshot.ObjectCapacity) {
            break;
        }

        if (slot.IsActive) {
            ++slotIndex;
            continue;
        }

        freeSlot = &slot;
        freeSlotIndex = slotIndex;
        break;
    }

    if (freeSlot == nullptr) {
        return object_registration_result_t::Failure(RecordFailure(ObjectStatus::CapacityExceeded));
    }

    const ObjectStatus typeStatus = RegisterTypeIfNeeded(descriptor.Type);
    if (typeStatus != ObjectStatus::Success) {
        return object_registration_result_t::Failure(RecordFailure(typeStatus));
    }

    if (freeSlot->Generation == INVALID_OBJECT_GENERATION) {
        freeSlot->Generation = 1U;
    }

    freeSlot->Type = descriptor.Type;
    freeSlot->ReferenceCount = descriptor.InitialReferenceCount;
    freeSlot->IsActive = true;
    ++_snapshot.AliveObjectCount;
    ++_snapshot.CreatedObjectCount;
    _snapshot.ReferencedObjectCount += descriptor.InitialReferenceCount;
    RecordSuccess();
    return object_registration_result_t::Success(object_handle_t{freeSlotIndex, freeSlot->Generation});
}

ObjectStatus ObjectRegistry::Validate(object_handle_t handle) {
    std::size_t slotIndex = 0U;
    const ObjectStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ObjectStatus::Success) {
        return RecordFailure(handleStatus);
    }

    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::Acquire(object_handle_t handle) {
    std::size_t slotIndex = 0U;
    const ObjectStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ObjectStatus::Success) {
        return RecordFailure(handleStatus);
    }

    object_slot_t& slot = _slots[slotIndex];
    if (slot.ReferenceCount == std::numeric_limits<std::uint32_t>::max()) {
        return RecordFailure(ObjectStatus::ReferenceCountOverflow);
    }

    ++slot.ReferenceCount;
    ++_snapshot.ReferencedObjectCount;
    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::Release(object_handle_t handle) {
    std::size_t slotIndex = 0U;
    const ObjectStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ObjectStatus::Success) {
        return RecordFailure(handleStatus);
    }

    object_slot_t& slot = _slots[slotIndex];
    if (slot.ReferenceCount == 0U) {
        return RecordFailure(ObjectStatus::NotAcquired);
    }

    --slot.ReferenceCount;
    --_snapshot.ReferencedObjectCount;
    ++_snapshot.ReleasedReferenceCount;
    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::Destroy(object_handle_t handle) {
    std::size_t slotIndex = 0U;
    const ObjectStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ObjectStatus::Success) {
        return RecordFailure(handleStatus);
    }

    object_slot_t& slot = _slots[slotIndex];
    if (slot.ReferenceCount != 0U) {
        return RecordFailure(ObjectStatus::StillReferenced);
    }

    slot.IsActive = false;
    slot.Type = object_type_id_t{};
    slot.ReferenceCount = 0U;
    AdvanceGeneration(slot);
    --_snapshot.AliveObjectCount;
    ++_snapshot.DestroyedObjectCount;
    RecordSuccess();
    return ObjectStatus::Success;
}

object_snapshot_t ObjectRegistry::Snapshot() const {
    return _snapshot;
}

ObjectStatus ObjectRegistry::RecordFailure(ObjectStatus status) {
    ++_snapshot.FailedOperationCount;
    _snapshot.LastStatus = status;
    return status;
}

void ObjectRegistry::RecordSuccess() {
    ++_snapshot.AcceptedOperationCount;
    _snapshot.LastStatus = ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::ResolveHandle(object_handle_t handle, std::size_t& outIndex) const {
    if (!handle.IsValid()) {
        return ObjectStatus::InvalidHandle;
    }

    if (handle.Slot >= _snapshot.ObjectCapacity) {
        return ObjectStatus::InvalidHandle;
    }

    const object_slot_t& slot = _slots[handle.Slot];
    if (slot.Generation == INVALID_OBJECT_GENERATION) {
        return ObjectStatus::InvalidHandle;
    }

    if (slot.Generation != handle.Generation) {
        return ObjectStatus::GenerationMismatch;
    }

    if (!slot.IsActive) {
        return ObjectStatus::InvalidHandle;
    }

    outIndex = handle.Slot;
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::RegisterTypeIfNeeded(object_type_id_t type) {
    if (HasType(type)) {
        return ObjectStatus::Success;
    }

    if (_snapshot.TypeCount >= _snapshot.TypeCapacity) {
        return ObjectStatus::CapacityExceeded;
    }

    _types[_snapshot.TypeCount] = type;
    ++_snapshot.TypeCount;
    return ObjectStatus::Success;
}

bool ObjectRegistry::HasType(object_type_id_t type) const {
    std::uint32_t index = 0U;
    for (const object_type_id_t& registeredType : _types) {
        if (index >= _snapshot.TypeCount) {
            return false;
        }

        if (registeredType.Value == type.Value) {
            return true;
        }

        ++index;
    }

    return false;
}

void ObjectRegistry::AdvanceGeneration(object_slot_t& slot) {
    if (slot.Generation == std::numeric_limits<std::uint32_t>::max()) {
        slot.Generation = 1U;
        return;
    }

    ++slot.Generation;
}
}
