#pragma once

#include <cstddef>

#include "yuengine/audio/audio_status.h"

namespace yuengine::audio {
struct AudioMixResult final {
    AUDIO_STATUS Status = AUDIO_STATUS::InvalidDescriptor;
    std::size_t FramesWritten = 0U;
};
}
