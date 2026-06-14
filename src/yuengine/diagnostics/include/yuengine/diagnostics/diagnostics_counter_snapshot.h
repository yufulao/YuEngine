#pragma once

#include <cstdint>

#include "yuengine/diagnostics/diagnostics_counter_id.h"

namespace yuengine::diagnostics {
struct diagnostics_counter_snapshot_t {
    diagnostics_counter_id_t Id;
    std::uint64_t Value;
    std::uint64_t SuccessfulUpdateCount;
};
}
