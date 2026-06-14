#pragma once

#include <cstdint>

#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/object/object_status.h"

namespace yuengine::object {
using memory::MEMORY_ACCOUNTING_STATUS;

struct object_snapshot_t final {
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
    MEMORY_ACCOUNTING_STATUS AllocationAccountingStatus;
    OBJECT_STATUS LastStatus;
};
}
