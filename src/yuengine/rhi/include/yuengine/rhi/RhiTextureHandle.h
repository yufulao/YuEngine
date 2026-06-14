#pragma once

#include <cstdint>

namespace yuengine::rhi {
struct RhiTextureHandle final {
    std::uint32_t Slot = 0U;
    std::uint32_t Generation = 0U;
};
}
