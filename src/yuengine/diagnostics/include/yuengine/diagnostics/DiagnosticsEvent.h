#pragma once

#include <cstdint>

#include "yuengine/diagnostics/DiagnosticsEventId.h"

namespace yuengine::diagnostics {
struct DiagnosticsEvent {
    DiagnosticsEventId Id;
    std::uint64_t Payload;
};
}
