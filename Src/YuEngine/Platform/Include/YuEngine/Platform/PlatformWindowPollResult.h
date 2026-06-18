// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowPollResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Platform/PlatformWindowStatus.h"

namespace yuengine::platform {
struct PlatformWindowPollResult {
    PlatformWindowStatus status = PlatformWindowStatus::NotCreated;
    std::size_t event_count = 0U;
    bool events_remaining = false;
    std::uint32_t dropped_event_count = 0U;
};
}
