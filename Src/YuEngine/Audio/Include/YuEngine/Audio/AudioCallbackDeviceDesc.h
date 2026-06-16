// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioCallbackDeviceDesc.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioBackendKind.h"
#include "YuEngine/Audio/AudioConstants.h"
#include "YuEngine/Audio/AudioSampleFormat.h"

namespace yuengine::audio {
struct AudioCallbackDeviceDesc final {
    static constexpr std::size_t MIN_BUFFER_COUNT = 2U;
    static constexpr std::size_t MAX_BUFFER_COUNT = 4U;
    static constexpr std::size_t MIN_FRAMES_PER_BUFFER = 16U;
    static constexpr std::size_t MAX_FRAMES_PER_BUFFER = MAX_OUTPUT_FRAMES;

    AudioBackendKind backend_kind = AudioBackendKind::Callback;
    AudioSampleFormat format = AudioSampleFormat::Signed16;
    std::uint32_t sample_rate = SAMPLE_RATE;
    std::uint16_t channel_count = CHANNEL_COUNT;
    std::size_t buffer_count = MIN_BUFFER_COUNT;
    std::size_t frames_per_buffer = MAX_OUTPUT_FRAMES;
};
}
