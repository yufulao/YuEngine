#pragma once

#include <cstddef>
#include <cstdint>

#include "yuengine/audio/audio_backend_kind.h"
#include "yuengine/audio/audio_constants.h"
#include "yuengine/audio/audio_sample_format.h"

namespace yuengine::audio {
struct AudioDeviceDesc final {
    AUDIO_BACKEND_KIND BackendKind = AUDIO_BACKEND_KIND::Test;
    AUDIO_SAMPLE_FORMAT Format = AUDIO_SAMPLE_FORMAT::S16;
    std::uint32_t SampleRate = SAMPLE_RATE;
    std::uint16_t ChannelCount = CHANNEL_COUNT;
    std::size_t SourceCapacity = MAX_SOURCES;
    std::size_t VoiceCapacity = MAX_VOICES;
};
}
