// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmStreamQueueRecord.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioPcmSamplePacketHandle.h"
#include "YuEngine/Audio/AudioPcmStreamQueueHandle.h"
#include "YuEngine/Audio/AudioSampleFormat.h"

namespace yuengine::audio {
struct AudioPcmStreamQueueRecord final {
    AudioPcmStreamQueueHandle handle{};
    std::uint32_t queue_id = 0U;
    AudioPcmSamplePacketHandle packet{};
    std::uint32_t packet_id = 0U;
    AudioSampleFormat format = AudioSampleFormat::Signed16;
    std::uint32_t sample_rate = 0U;
    std::uint16_t channel_count = 0U;
    std::size_t first_frame = 0U;
    std::size_t frame_count = 0U;
    std::size_t drained_frame_count = 0U;
    std::size_t remaining_frame_count = 0U;
    std::size_t interleaved_sample_count = 0U;
    std::size_t byte_count = 0U;
    std::size_t chunk_frame_count = 0U;
    bool is_active = false;
};
}
