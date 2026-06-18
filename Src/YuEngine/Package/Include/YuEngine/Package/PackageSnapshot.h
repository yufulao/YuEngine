// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Include/YuEngine/Package/PackageSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Package/PackageStatus.h"

namespace yuengine::package {
using memory::MemoryAccountingStatus;

struct PackageSnapshot final {
    std::uint32_t manifest_capacity;
    std::uint32_t entry_capacity;
    std::uint32_t dependency_edge_capacity;
    std::uint32_t load_plan_record_capacity;
    std::uint32_t manifest_count;
    std::uint32_t entry_count;
    std::uint32_t dependency_edge_count;
    std::uint32_t dependency_validation_count;
    std::uint32_t load_plan_resolve_count;
    std::uint32_t last_load_plan_record_count;
    std::uint32_t rejected_operation_count;
    MemoryAccountingStatus allocation_accounting_status;
    PackageStatus last_status;
};
}
