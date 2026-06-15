// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioCapabilities.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioBackendKind.h"
#include "YuEngine/Audio/AudioSampleFormat.h"

namespace yuengine::audio {
struct AudioCapabilities final {
    AudioBackendKind backend_kind = AudioBackendKind::Test;
    AudioSampleFormat format = AudioSampleFormat::Signed16;
    std::uint32_t sample_rate = 0U;
    std::uint16_t channel_count = 0U;
    std::size_t source_capacity = 0U;
    std::size_t voice_capacity = 0U;
    std::size_t max_source_frames = 0U;
    std::size_t max_output_frames = 0U;
    bool supports_deterministic_mix = false;
};
}
