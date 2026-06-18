// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiSampledTextureBinding.h

#pragma once

#include <cstdint>

#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rhi {
struct RhiSampledTextureBinding final {
    RhiTextureHandle texture{};
    std::uint32_t slot = 0U;
};
}
