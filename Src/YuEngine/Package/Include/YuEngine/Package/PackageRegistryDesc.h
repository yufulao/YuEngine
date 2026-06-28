// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Include/YuEngine/Package/PackageRegistryDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Package/PackageConstants.h"

namespace yuengine::package {
struct PackageRegistryDesc final {
    std::uint32_t manifest_capacity = MAX_PACKAGE_MANIFEST_COUNT;
    std::uint32_t entry_capacity = MAX_PACKAGE_ENTRY_COUNT;
    std::uint32_t dependency_edge_capacity = MAX_PACKAGE_DEPENDENCY_EDGE_COUNT;
    std::uint32_t load_plan_record_capacity = MAX_LOAD_PLAN_RECORD_COUNT;
    std::uint64_t load_plan_archive_byte_budget = MAX_LOAD_PLAN_ARCHIVE_BYTE_BUDGET;
};
}
