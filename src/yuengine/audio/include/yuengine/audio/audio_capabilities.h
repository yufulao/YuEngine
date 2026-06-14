#pragma once

#include <cstddef>
#include <cstdint>

#include "yuengine/audio/audio_backend_kind.h"
#include "yuengine/audio/audio_sample_format.h"

namespace yuengine::audio {
struct audio_capabilities_t final {
    AudioBackendKind BackendKind = AudioBackendKind::Test;
    AudioSampleFormat Format = AudioSampleFormat::S16;
    std::uint32_t SampleRate = 0U;
    std::uint16_t ChannelCount = 0U;
    std::size_t SourceCapacity = 0U;
    std::size_t VoiceCapacity = 0U;
    std::size_t MaxSourceFrames = 0U;
    std::size_t MaxOutputFrames = 0U;
    bool SupportsDeterministicMix = false;
};
}
