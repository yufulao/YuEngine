#pragma once

#include <cstdint>

namespace yuengine::audio {
struct audio_source_id_t final {
    std::uint32_t Slot = 0U;
    std::uint32_t Generation = 0U;
};
}
