#pragma once

#include <cstdint>

#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/package/package_status.h"

namespace yuengine::package {
using memory::MEMORY_ACCOUNTING_STATUS;

struct package_snapshot_t final {
    std::uint32_t ManifestCapacity;
    std::uint32_t EntryCapacity;
    std::uint32_t DependencyEdgeCapacity;
    std::uint32_t LoadPlanRecordCapacity;
    std::uint32_t ManifestCount;
    std::uint32_t EntryCount;
    std::uint32_t DependencyEdgeCount;
    std::uint32_t DependencyValidationCount;
    std::uint32_t LoadPlanResolveCount;
    std::uint32_t LastLoadPlanRecordCount;
    std::uint32_t RejectedOperationCount;
    MEMORY_ACCOUNTING_STATUS AllocationAccountingStatus;
    PACKAGE_STATUS LastStatus;
};
}
