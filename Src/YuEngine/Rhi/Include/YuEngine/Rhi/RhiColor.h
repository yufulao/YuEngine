// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiColor.h

#pragma once

#include <cstdint>

namespace yuengine::rhi {
struct RhiColor final {
    std::uint8_t r = 0U;
    std::uint8_t g = 0U;
    std::uint8_t b = 0U;
    std::uint8_t a = 0U;
};
}
