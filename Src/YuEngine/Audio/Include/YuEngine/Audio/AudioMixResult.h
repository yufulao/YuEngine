// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioMixResult.h

#pragma once

#include <cstddef>

#include "YuEngine/Audio/AudioStatus.h"

namespace yuengine::audio {
struct AudioMixResult final {
    AudioStatus status = AudioStatus::InvalidDescriptor;
    std::size_t frames_written = 0U;
};
}
