// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiExtent2D.h

#pragma once

#include <cstdint>

namespace yuengine::rhi {
struct RhiExtent2D final {
    std::uint16_t width = 0U;
    std::uint16_t height = 0U;
};
}
