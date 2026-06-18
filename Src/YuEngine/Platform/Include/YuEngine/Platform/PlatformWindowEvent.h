// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowEvent.h

#pragma once

#include <cstdint>

#include "YuEngine/Platform/PlatformWindowEventType.h"

namespace yuengine::platform {
struct PlatformWindowEvent {
    PlatformWindowEventType type = PlatformWindowEventType::None;
    std::uint32_t client_width = 0U;
    std::uint32_t client_height = 0U;
    std::uint32_t raw_code = 0U;
    std::int32_t pointer_x = 0;
    std::int32_t pointer_y = 0;
    std::int32_t wheel_delta = 0;
};
}
