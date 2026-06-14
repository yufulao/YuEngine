#include "yuengine/object/ObjectRegistry.h"

#include <limits>

using MemoryAccountingStatus = yuengine::memory::MemoryAccountingStatus;

namespace yuengine::object
{
namespace
{
std::uint32_t ClampCapacity(std::uint32_t requestedCapacity, std::uint32_t maximumCapacity)
{
    if (requestedCapacity > maximumCapacity)
    {
        return maximumCapacity;
    }

    return requestedCapacity;
}
}

ObjectRegistry::ObjectRegistry()
    : ObjectRegistry(ObjectRegistryDesc{})
{
}

ObjectRegistry::ObjectRegistry(ObjectRegistryDesc desc)
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
          ObjectStatus::Success}
{
}

ObjectRegistrationResult ObjectRegistry::CreateSyntheticObject(const ObjectDescriptor& descriptor)
{
    if (!descriptor.Type.IsValid())
    {
        return ObjectRegistrationResult::Failure(RecordFailure(ObjectStatus::InvalidType));
    }

    if (_snapshot.AliveObjectCount >= _snapshot.ObjectCapacity)
    {
        return ObjectRegistrationResult::Failure(RecordFailure(ObjectStatus::CapacityExceeded));
    }

    ObjectSlot* freeSlot = nullptr;
    std::uint32_t freeSlotIndex = 0U;
    std::uint32_t slotIndex = 0U;
    for (ObjectSlot& slot : _slots)
    {
        if (slotIndex >= _snapshot.ObjectCapacity)
        {
            break;
        }

        if (slot.IsActive)
        {
            ++slotIndex;
            continue;
        }

        freeSlot = &slot;
        freeSlotIndex = slotIndex;
        break;
    }

    if (freeSlot == nullptr)
    {
        return ObjectRegistrationResult::Failure(RecordFailure(ObjectStatus::CapacityExceeded));
    }

    const ObjectStatus typeStatus = RegisterTypeIfNeeded(descriptor.Type);
    if (typeStatus != ObjectStatus::Success)
    {
        return ObjectRegistrationResult::Failure(RecordFailure(typeStatus));
    }

    if (freeSlot->Generation == INVALID_OBJECT_GENERATION)
    {
        freeSlot->Generation = 1U;
    }

    freeSlot->Type = descriptor.Type;
    freeSlot->ReferenceCount = descriptor.InitialReferenceCount;
    freeSlot->IsActive = true;
    ++_snapshot.AliveObjectCount;
    ++_snapshot.CreatedObjectCount;
    _snapshot.ReferencedObjectCount += descriptor.InitialReferenceCount;
    RecordSuccess();
    return ObjectRegistrationResult::Success(ObjectHandle{freeSlotIndex, freeSlot->Generation});
}

ObjectStatus ObjectRegistry::Validate(ObjectHandle handle)
{
    std::size_t slotIndex = 0U;
    const ObjectStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ObjectStatus::Success)
    {
        return RecordFailure(handleStatus);
    }

    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::Acquire(ObjectHandle handle)
{
    std::size_t slotIndex = 0U;
    const ObjectStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ObjectStatus::Success)
    {
        return RecordFailure(handleStatus);
    }

    ObjectSlot& slot = _slots[slotIndex];
    if (slot.ReferenceCount == std::numeric_limits<std::uint32_t>::max())
    {
        return RecordFailure(ObjectStatus::ReferenceCountOverflow);
    }

    ++slot.ReferenceCount;
    ++_snapshot.ReferencedObjectCount;
    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::Release(ObjectHandle handle)
{
    std::size_t slotIndex = 0U;
    const ObjectStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ObjectStatus::Success)
    {
        return RecordFailure(handleStatus);
    }

    ObjectSlot& slot = _slots[slotIndex];
    if (slot.ReferenceCount == 0U)
    {
        return RecordFailure(ObjectStatus::NotAcquired);
    }

    --slot.ReferenceCount;
    --_snapshot.ReferencedObjectCount;
    ++_snapshot.ReleasedReferenceCount;
    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::Destroy(ObjectHandle handle)
{
    std::size_t slotIndex = 0U;
    const ObjectStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ObjectStatus::Success)
    {
        return RecordFailure(handleStatus);
    }

    ObjectSlot& slot = _slots[slotIndex];
    if (slot.ReferenceCount != 0U)
    {
        return RecordFailure(ObjectStatus::StillReferenced);
    }

    slot.IsActive = false;
    slot.Type = ObjectTypeId{};
    slot.ReferenceCount = 0U;
    AdvanceGeneration(slot);
    --_snapshot.AliveObjectCount;
    ++_snapshot.DestroyedObjectCount;
    RecordSuccess();
    return ObjectStatus::Success;
}

ObjectSnapshot ObjectRegistry::Snapshot() const
{
    return _snapshot;
}

ObjectStatus ObjectRegistry::RecordFailure(ObjectStatus status)
{
    ++_snapshot.FailedOperationCount;
    _snapshot.LastStatus = status;
    return status;
}

void ObjectRegistry::RecordSuccess()
{
    ++_snapshot.AcceptedOperationCount;
    _snapshot.LastStatus = ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::ResolveHandle(ObjectHandle handle, std::size_t& outIndex) const
{
    if (!handle.IsValid())
    {
        return ObjectStatus::InvalidHandle;
    }

    if (handle.Slot >= _snapshot.ObjectCapacity)
    {
        return ObjectStatus::InvalidHandle;
    }

    const ObjectSlot& slot = _slots[handle.Slot];
    if (slot.Generation == INVALID_OBJECT_GENERATION)
    {
        return ObjectStatus::InvalidHandle;
    }

    if (slot.Generation != handle.Generation)
    {
        return ObjectStatus::GenerationMismatch;
    }

    if (!slot.IsActive)
    {
        return ObjectStatus::InvalidHandle;
    }

    outIndex = handle.Slot;
    return ObjectStatus::Success;
}

ObjectStatus ObjectRegistry::RegisterTypeIfNeeded(ObjectTypeId type)
{
    if (HasType(type))
    {
        return ObjectStatus::Success;
    }

    if (_snapshot.TypeCount >= _snapshot.TypeCapacity)
    {
        return ObjectStatus::CapacityExceeded;
    }

    _types[_snapshot.TypeCount] = type;
    ++_snapshot.TypeCount;
    return ObjectStatus::Success;
}

bool ObjectRegistry::HasType(ObjectTypeId type) const
{
    std::uint32_t index = 0U;
    for (const ObjectTypeId& registeredType : _types)
    {
        if (index >= _snapshot.TypeCount)
        {
            return false;
        }

        if (registeredType.Value == type.Value)
        {
            return true;
        }

        ++index;
    }

    return false;
}

void ObjectRegistry::AdvanceGeneration(ObjectSlot& slot)
{
    if (slot.Generation == std::numeric_limits<std::uint32_t>::max())
    {
        slot.Generation = 1U;
        return;
    }

    ++slot.Generation;
}
}
