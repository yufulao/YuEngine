// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmSamplePacketRecord.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioPcmSamplePacketHandle.h"
#include "YuEngine/Audio/AudioSampleFormat.h"

namespace yuengine::audio {
struct AudioPcmSamplePacketRecord final {
    AudioPcmSamplePacketHandle handle{};
    std::uint32_t packet_id = 0U;
    AudioSampleFormat format = AudioSampleFormat::Signed16;
    std::uint32_t sample_rate = 0U;
    std::uint16_t channel_count = 0U;
    std::size_t frame_count = 0U;
    std::size_t interleaved_sample_count = 0U;
    std::size_t byte_count = 0U;
    bool is_active = false;
};
}
