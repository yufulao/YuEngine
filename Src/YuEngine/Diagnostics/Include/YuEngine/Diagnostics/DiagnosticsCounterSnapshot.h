// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DiagnosticsCounterSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsCounterId.h"

namespace yuengine::diagnostics {
struct DiagnosticsCounterSnapshot {
    DiagnosticsCounterId id;
    std::uint64_t value;
    std::uint64_t successful_update_count;
};
}
