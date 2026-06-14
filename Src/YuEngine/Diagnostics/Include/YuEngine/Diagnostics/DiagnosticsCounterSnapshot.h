#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsCounterId.h"

namespace yuengine::diagnostics {
struct DiagnosticsCounterSnapshot {
    DiagnosticsCounterId Id;
    std::uint64_t Value;
    std::uint64_t SuccessfulUpdateCount;
};
}
