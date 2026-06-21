// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiBlendStateDesc.h

#pragma once

#include <cstdint>

namespace yuengine::rhi {
enum class RhiBlendMode {
    Opaque,
    AlphaOver
};

struct RhiBlendStateDesc final {
    RhiBlendMode mode = RhiBlendMode::Opaque;
    std::uint8_t constant_alpha = 255U;
};
}
