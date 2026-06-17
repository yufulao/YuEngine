// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmSamplePacketRequest.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioConstants.h"
#include "YuEngine/Audio/AudioSampleFormat.h"

namespace yuengine::audio {
struct AudioPcmSamplePacketRequest final {
    std::uint32_t packet_id = 0U;
    AudioSampleFormat format = AudioSampleFormat::Signed16;
    std::uint32_t sample_rate = SAMPLE_RATE;
    std::uint16_t channel_count = CHANNEL_COUNT;
    std::size_t frame_count = 0U;
    std::size_t interleaved_sample_count = 0U;
    std::size_t byte_count = 0U;
};
}
