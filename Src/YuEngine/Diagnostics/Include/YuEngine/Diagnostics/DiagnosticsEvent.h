#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsEventId.h"

namespace yuengine::diagnostics {
struct DiagnosticsEvent {
    DiagnosticsEventId Id;
    std::uint64_t Payload;
};
}
