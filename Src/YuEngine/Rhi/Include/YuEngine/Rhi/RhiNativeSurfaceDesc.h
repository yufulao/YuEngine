// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiNativeSurfaceDesc.h

#pragma once

#include <cstdint>

namespace yuengine::rhi {
struct RhiNativeSurfaceDesc final {
    std::uintptr_t window_value = 0U;
    std::uintptr_t instance_value = 0U;
    bool valid = false;
};
}
