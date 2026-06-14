#pragma once

#include <array>
#include <cstdint>

#include "yuengine/package/package_constants.h"
#include "yuengine/package/package_load_plan_record.h"

namespace yuengine::package {
struct package_load_plan_t final {
    std::array<package_load_plan_record_t, MAX_LOAD_PLAN_RECORD_COUNT> Records;
    std::uint32_t RecordCount = 0U;
};
}
