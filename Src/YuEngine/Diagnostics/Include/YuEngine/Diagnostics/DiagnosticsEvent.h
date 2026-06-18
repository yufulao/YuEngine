// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DiagnosticsEvent.h

#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsEventId.h"

namespace yuengine::diagnostics {
struct DiagnosticsEvent {
    DiagnosticsEventId id;
    std::uint64_t payload;
};
}
