// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiShaderModuleHandle.h

#pragma once

#include <cstdint>

namespace yuengine::rhi {
struct RhiShaderModuleHandle final {
    std::uint32_t slot = 0U;
    std::uint32_t generation = 0U;
};
}
