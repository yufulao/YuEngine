#pragma once

#include <cstddef>

#include "yuengine/audio/audio_status.h"

namespace yuengine::audio {
struct AudioMixResult final {
    AudioStatus Status = AudioStatus::InvalidDescriptor;
    std::size_t FramesWritten = 0U;
};
}
