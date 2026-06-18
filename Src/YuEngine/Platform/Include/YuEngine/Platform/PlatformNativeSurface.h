// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformNativeSurface.h

#pragma once

#include <cstdint>

namespace yuengine::platform {
struct PlatformNativeSurface {
    std::uintptr_t window_value = 0U;
    std::uintptr_t instance_value = 0U;
    bool valid = false;
};
}
