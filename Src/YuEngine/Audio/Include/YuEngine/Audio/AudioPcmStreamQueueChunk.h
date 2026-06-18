// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmStreamQueueChunk.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioPcmSamplePacketHandle.h"
#include "YuEngine/Audio/AudioPcmStreamQueueHandle.h"

namespace yuengine::audio {
struct AudioPcmStreamQueueChunk final {
    AudioPcmStreamQueueHandle queue{};
    AudioPcmSamplePacketHandle packet{};
    std::uint32_t queue_id = 0U;
    std::uint32_t packet_id = 0U;
    std::size_t chunk_index = 0U;
    std::size_t first_frame = 0U;
    std::size_t frame_count = 0U;
    std::size_t first_interleaved_sample = 0U;
    std::size_t interleaved_sample_count = 0U;
    std::size_t byte_count = 0U;
    bool is_final_chunk = false;
};
}
