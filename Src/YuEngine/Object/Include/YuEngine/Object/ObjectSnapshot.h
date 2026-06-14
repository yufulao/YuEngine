#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectStatus.h"

namespace yuengine::object {
using memory::MemoryAccountingStatus;

struct ObjectSnapshot final {
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
    MemoryAccountingStatus AllocationAccountingStatus;
    ObjectStatus LastStatus;
};
}
