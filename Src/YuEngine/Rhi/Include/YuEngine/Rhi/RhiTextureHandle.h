#pragma once

#include <cstdint>

namespace yuengine::rhi {
struct RhiTextureHandle final {
    std::uint32_t slot = 0U;
    std::uint32_t generation = 0U;
};
}
