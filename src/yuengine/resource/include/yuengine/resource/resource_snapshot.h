#pragma once

#include <cstdint>

#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/resource/resource_status.h"

namespace yuengine::resource {
struct ResourceSnapshot final {
    std::uint32_t ResourceCapacity;
    std::uint32_t TypeCapacity;
    std::uint32_t DependencyEdgeCapacity;
    std::uint32_t RegisteredResourceCount;
    std::uint32_t TypeCount;
    std::uint64_t AcquiredHandleCount;
    std::uint64_t ReleasedHandleCount;
    std::uint32_t RetiredResourceCount;
    std::uint32_t DependencyEdgeCount;
    std::uint32_t DependencyValidationCount;
    std::uint32_t FailedOperationCount;
    yuengine::memory::MemoryAccountingStatus AllocationAccountingStatus;
    ResourceStatus LastStatus;
};
}
