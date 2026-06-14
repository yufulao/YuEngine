#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Package/PackageStatus.h"

namespace yuengine::package {
using memory::MemoryAccountingStatus;

struct PackageSnapshot final {
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
    MemoryAccountingStatus AllocationAccountingStatus;
    PackageStatus LastStatus;
};
}
