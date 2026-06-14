#pragma once

#include <array>
#include <cstdint>

#include "yuengine/package/PackageConstants.h"
#include "yuengine/package/PackageLoadPlanRecord.h"

namespace yuengine::package {
struct PackageLoadPlan final {
    std::array<PackageLoadPlanRecord, MAX_LOAD_PLAN_RECORD_COUNT> Records;
    std::uint32_t RecordCount = 0U;
};
}
