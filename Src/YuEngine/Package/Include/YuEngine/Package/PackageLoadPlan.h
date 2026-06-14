#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Package/PackageConstants.h"
#include "YuEngine/Package/PackageLoadPlanRecord.h"

namespace yuengine::package {
struct PackageLoadPlan final {
    std::array<PackageLoadPlanRecord, MAX_LOAD_PLAN_RECORD_COUNT> records;
    std::uint32_t record_count = 0U;
};
}
