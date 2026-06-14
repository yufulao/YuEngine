#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioBackendKind.h"
#include "YuEngine/Audio/AudioConstants.h"
#include "YuEngine/Audio/AudioSampleFormat.h"

namespace yuengine::audio {
struct AudioDeviceDesc final {
    AudioBackendKind backend_kind = AudioBackendKind::Test;
    AudioSampleFormat format = AudioSampleFormat::Signed16;
    std::uint32_t sample_rate = SAMPLE_RATE;
    std::uint16_t channel_count = CHANNEL_COUNT;
    std::size_t source_capacity = MAX_SOURCES;
    std::size_t voice_capacity = MAX_VOICES;
};
}
