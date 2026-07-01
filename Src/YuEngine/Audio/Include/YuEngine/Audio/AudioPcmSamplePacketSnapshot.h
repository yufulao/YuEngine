// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmSamplePacketSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioPcmSamplePacketOperation.h"
#include "YuEngine/Audio/AudioSampleFormat.h"
#include "YuEngine/Audio/AudioStatus.h"

namespace yuengine::audio {
struct AudioPcmSamplePacketSnapshot final {
    std::size_t packet_capacity = 0U;
    std::size_t active_packet_count = 0U;
    std::uint64_t created_packet_count = 0U;
    std::uint64_t queried_packet_count = 0U;
    std::uint64_t released_packet_count = 0U;
    std::uint64_t rejected_packet_count = 0U;
    std::uint64_t duplicate_packet_rejected_count = 0U;
    std::uint64_t stale_packet_rejected_count = 0U;
    std::uint64_t capacity_rejected_count = 0U;
    std::size_t last_required_packet_count = 0U;
    std::size_t last_failed_packet_capacity = 0U;
    std::size_t last_failed_active_packet_count = 0U;
    std::uint32_t last_failed_packet_id = 0U;
    std::size_t last_failed_frame_count = 0U;
    std::uint32_t last_failed_sample_rate = 0U;
    std::uint16_t last_failed_channel_count = 0U;
    AudioSampleFormat last_failed_format = AudioSampleFormat::Signed16;
    AudioStatus last_status = AudioStatus::NotInitialized;
    AudioPcmSamplePacketOperation last_operation = AudioPcmSamplePacketOperation::None;
};
}
