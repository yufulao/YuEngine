// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Include/YuEngine/Package/PackageLoadPlan.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Package/PackageConstants.h"
#include "YuEngine/Package/PackageLoadPlanRecord.h"

namespace yuengine::package {
struct PackageLoadPlan final {
    std::array<PackageLoadPlanRecord, MAX_LOAD_PLAN_RECORD_COUNT> records;
    std::uint32_t record_count = 0U;
    std::uint64_t archive_byte_count = 0ULL;
};
}
