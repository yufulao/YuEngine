#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace yuengine::audio {
struct audio_source_slot_t final {
    bool IsActive = false;
    std::uint32_t Generation = 1U;
    std::size_t FrameCount = 0U;
    std::vector<std::int16_t> Samples{};
};
}
