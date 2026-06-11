#pragma once

#include <cstdint>
#include <limits>

#include "yuengine/memory/MemoryAccountingStatus.h"

namespace yuengine::object
{
constexpr std::uint32_t MAX_OBJECT_COUNT = 64U;
constexpr std::uint32_t MAX_OBJECT_TYPE_COUNT = 16U;
constexpr std::uint32_t INVALID_OBJECT_GENERATION = 0U;
constexpr std::uint32_t INVALID_OBJECT_SLOT = std::numeric_limits<std::uint32_t>::max();

enum class ObjectStatus
{
    Success,
    InvalidType,
    CapacityExceeded,
    InvalidHandle,
    GenerationMismatch,
    NotAcquired,
    ReferenceCountOverflow,
    StillReferenced
};

struct ObjectTypeId final
{
    std::uint32_t Value = 0U;

    bool IsValid() const
    {
        return Value != 0U;
    }
};

struct ObjectHandle final
{
    std::uint32_t Slot = INVALID_OBJECT_SLOT;
    std::uint32_t Generation = INVALID_OBJECT_GENERATION;

    bool IsValid() const
    {
        if (Slot == INVALID_OBJECT_SLOT)
        {
            return false;
        }

        return Generation != INVALID_OBJECT_GENERATION;
    }
};

struct ObjectDescriptor final
{
    ObjectTypeId Type;
    std::uint32_t InitialReferenceCount = 0U;
};

struct ObjectRegistryDesc final
{
    std::uint32_t ObjectCapacity = MAX_OBJECT_COUNT;
    std::uint32_t TypeCapacity = MAX_OBJECT_TYPE_COUNT;
};

struct ObjectSnapshot final
{
    std::uint32_t ObjectCapacity;
    std::uint32_t TypeCapacity;
    std::uint32_t TypeCount;
    std::uint32_t AliveObjectCount;
    std::uint32_t DestroyedObjectCount;
    std::uint64_t CreatedObjectCount;
    std::uint64_t ReferencedObjectCount;
    std::uint64_t ReleasedReferenceCount;
    std::uint32_t AcceptedOperationCount;
    std::uint32_t FailedOperationCount;
    yuengine::memory::MemoryAccountingStatus AllocationAccountingStatus;
    ObjectStatus LastStatus;
};

struct ObjectRegistrationResult final
{
    ObjectStatus Status;
    ObjectHandle Handle;

    static ObjectRegistrationResult Success(ObjectHandle handle)
    {
        return ObjectRegistrationResult{ObjectStatus::Success, handle};
    }

    static ObjectRegistrationResult Failure(ObjectStatus status)
    {
        return ObjectRegistrationResult{status, ObjectHandle{}};
    }

    bool Succeeded() const
    {
        return Status == ObjectStatus::Success;
    }
};
}
