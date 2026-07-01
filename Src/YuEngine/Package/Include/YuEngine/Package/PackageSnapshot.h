// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Include/YuEngine/Package/PackageSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Package/PackageEntryId.h"
#include "YuEngine/Package/PackageId.h"
#include "YuEngine/Package/PackageStatus.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::package {
using memory::MemoryAccountingStatus;
using resource::ResourceLogicalKey;
using resource::ResourceTypeId;

struct PackageSnapshot final {
    std::uint32_t manifest_capacity;
    std::uint32_t entry_capacity;
    std::uint32_t dependency_edge_capacity;
    std::uint32_t load_plan_record_capacity;
    std::uint64_t load_plan_archive_byte_budget;
    std::uint32_t manifest_count;
    std::uint32_t entry_count;
    std::uint32_t required_manifest_record_count = 0U;
    std::uint32_t required_entry_record_count = 0U;
    std::uint32_t last_failed_manifest_index = 0U;
    PackageId last_failed_package{};
    std::uint32_t last_failed_entry_index = 0U;
    PackageEntryId last_failed_entry{};
    std::uint32_t dependency_edge_count;
    std::uint32_t dependency_validation_count;
    std::uint32_t load_plan_resolve_count;
    std::uint32_t last_load_plan_record_count;
    std::uint32_t required_load_plan_record_count = 0U;
    std::uint64_t last_load_plan_archive_byte_count;
    std::uint32_t accepted_operation_count;
    std::uint32_t rejected_operation_count;
    MemoryAccountingStatus allocation_accounting_status;
    PackageStatus last_status;
    PackageId last_failed_load_plan_package{};
    PackageEntryId last_failed_load_plan_entry_id{};
    ResourceTypeId last_failed_load_plan_resource_type{};
    ResourceLogicalKey last_failed_load_plan_resource_key{};
    std::uint32_t last_failed_load_plan_record_capacity = 0U;
    std::uint32_t last_failed_load_plan_record_count = 0U;
    std::uint32_t last_required_dependency_edge_count = 0U;
    PackageStatus last_registration_capacity_failure_kind = PackageStatus::Success;
    std::uint32_t last_failed_manifest_capacity = 0U;
    std::uint32_t last_failed_entry_capacity = 0U;
    std::uint32_t last_failed_dependency_edge_capacity = 0U;
    std::uint32_t last_failed_manifest_count = 0U;
    std::uint32_t last_failed_entry_count = 0U;
    std::uint32_t last_failed_dependency_edge_count = 0U;
    std::uint32_t last_failed_dependency_edge_index = 0U;
    PackageEntryId last_failed_dependency{};
};
}
