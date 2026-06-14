#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace yuengine::audio {
struct AudioSourceSlot final {
    bool is_active = false;
    std::uint32_t generation = 1U;
    std::size_t frame_count = 0U;
    std::vector<std::int16_t> samples{};
};
}
