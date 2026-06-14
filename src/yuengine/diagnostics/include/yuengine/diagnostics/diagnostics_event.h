#pragma once

#include <cstdint>

#include "yuengine/diagnostics/diagnostics_event_id.h"

namespace yuengine::diagnostics {
struct diagnostics_event_t {
    diagnostics_event_id_t Id;
    std::uint64_t Payload;
};
}
