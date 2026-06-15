// Module: YuEngine Diagnostics
// File: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DiagnosticsEvent.h

#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsEventId.h"

namespace yuengine::diagnostics {
struct DiagnosticsEvent {
    DiagnosticsEventId id;
    std::uint64_t payload;
};
}
