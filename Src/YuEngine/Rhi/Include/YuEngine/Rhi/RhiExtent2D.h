#pragma once

#include <cstdint>

namespace yuengine::rhi {
struct RhiExtent2D final {
    std::uint16_t Width = 0U;
    std::uint16_t Height = 0U;
};
}
