#include "yuengine/object/object_registry.h"

#include <limits>

using yuengine::memory::MEMORY_ACCOUNTING_STATUS;

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
          MEMORY_ACCOUNTING_STATUS::ExplicitlyTrackedOnly,
          OBJECT_STATUS::Success} {
}

object_registration_result_t ObjectRegistry::CreateSyntheticObject(const object_descriptor_t& descriptor) {
    if (!descriptor.Type.IsValid()) {
        return object_registration_result_t::Failure(RecordFailure(OBJECT_STATUS::InvalidType));
    }

    if (_snapshot.AliveObjectCount >= _snapshot.ObjectCapacity) {
        return object_registration_result_t::Failure(RecordFailure(OBJECT_STATUS::CapacityExceeded));
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
        return object_registration_result_t::Failure(RecordFailure(OBJECT_STATUS::CapacityExceeded));
    }

    const OBJECT_STATUS typeStatus = RegisterTypeIfNeeded(descriptor.Type);
    if (typeStatus != OBJECT_STATUS::Success) {
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

OBJECT_STATUS ObjectRegistry::Validate(object_handle_t handle) {
    std::size_t slotIndex = 0U;
    const OBJECT_STATUS handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != OBJECT_STATUS::Success) {
        return RecordFailure(handleStatus);
    }

    RecordSuccess();
    return OBJECT_STATUS::Success;
}

OBJECT_STATUS ObjectRegistry::Acquire(object_handle_t handle) {
    std::size_t slotIndex = 0U;
    const OBJECT_STATUS handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != OBJECT_STATUS::Success) {
        return RecordFailure(handleStatus);
    }

    object_slot_t& slot = _slots[slotIndex];
    if (slot.ReferenceCount == std::numeric_limits<std::uint32_t>::max()) {
        return RecordFailure(OBJECT_STATUS::ReferenceCountOverflow);
    }

    ++slot.ReferenceCount;
    ++_snapshot.ReferencedObjectCount;
    RecordSuccess();
    return OBJECT_STATUS::Success;
}

OBJECT_STATUS ObjectRegistry::Release(object_handle_t handle) {
    std::size_t slotIndex = 0U;
    const OBJECT_STATUS handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != OBJECT_STATUS::Success) {
        return RecordFailure(handleStatus);
    }

    object_slot_t& slot = _slots[slotIndex];
    if (slot.ReferenceCount == 0U) {
        return RecordFailure(OBJECT_STATUS::NotAcquired);
    }

    --slot.ReferenceCount;
    --_snapshot.ReferencedObjectCount;
    ++_snapshot.ReleasedReferenceCount;
    RecordSuccess();
    return OBJECT_STATUS::Success;
}

OBJECT_STATUS ObjectRegistry::Destroy(object_handle_t handle) {
    std::size_t slotIndex = 0U;
    const OBJECT_STATUS handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != OBJECT_STATUS::Success) {
        return RecordFailure(handleStatus);
    }

    object_slot_t& slot = _slots[slotIndex];
    if (slot.ReferenceCount != 0U) {
        return RecordFailure(OBJECT_STATUS::StillReferenced);
    }

    slot.IsActive = false;
    slot.Type = object_type_id_t{};
    slot.ReferenceCount = 0U;
    AdvanceGeneration(slot);
    --_snapshot.AliveObjectCount;
    ++_snapshot.DestroyedObjectCount;
    RecordSuccess();
    return OBJECT_STATUS::Success;
}

object_snapshot_t ObjectRegistry::Snapshot() const {
    return _snapshot;
}

OBJECT_STATUS ObjectRegistry::RecordFailure(OBJECT_STATUS status) {
    ++_snapshot.FailedOperationCount;
    _snapshot.LastStatus = status;
    return status;
}

void ObjectRegistry::RecordSuccess() {
    ++_snapshot.AcceptedOperationCount;
    _snapshot.LastStatus = OBJECT_STATUS::Success;
}

OBJECT_STATUS ObjectRegistry::ResolveHandle(object_handle_t handle, std::size_t& outIndex) const {
    if (!handle.IsValid()) {
        return OBJECT_STATUS::InvalidHandle;
    }

    if (handle.Slot >= _snapshot.ObjectCapacity) {
        return OBJECT_STATUS::InvalidHandle;
    }

    const object_slot_t& slot = _slots[handle.Slot];
    if (slot.Generation == INVALID_OBJECT_GENERATION) {
        return OBJECT_STATUS::InvalidHandle;
    }

    if (slot.Generation != handle.Generation) {
        return OBJECT_STATUS::GenerationMismatch;
    }

    if (!slot.IsActive) {
        return OBJECT_STATUS::InvalidHandle;
    }

    outIndex = handle.Slot;
    return OBJECT_STATUS::Success;
}

OBJECT_STATUS ObjectRegistry::RegisterTypeIfNeeded(object_type_id_t type) {
    if (HasType(type)) {
        return OBJECT_STATUS::Success;
    }

    if (_snapshot.TypeCount >= _snapshot.TypeCapacity) {
        return OBJECT_STATUS::CapacityExceeded;
    }

    _types[_snapshot.TypeCount] = type;
    ++_snapshot.TypeCount;
    return OBJECT_STATUS::Success;
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
