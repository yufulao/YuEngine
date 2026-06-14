#pragma once

#include <cstddef>
#include <cstdint>

#include "yuengine/audio/audio_source_id.h"

namespace yuengine::audio {
struct AudioVoiceSlot final {
    bool IsActive = false;
    std::uint32_t Generation = 1U;
    AudioSourceId Source{};
    std::size_t CursorFrame = 0U;
    std::uint32_t GainQ15 = 0U;
};
}
