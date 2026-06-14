#pragma once

#include <cstdint>

#include "yuengine/diagnostics/diagnostics_event_id.h"

namespace yuengine::diagnostics {
struct DiagnosticsEvent {
    DiagnosticsEventId Id;
    std::uint64_t Payload;
};
}
