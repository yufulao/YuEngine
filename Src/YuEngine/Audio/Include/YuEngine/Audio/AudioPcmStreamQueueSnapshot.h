// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmStreamQueueSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioPcmStreamQueueOperation.h"
#include "YuEngine/Audio/AudioPcmStreamQueueStatus.h"

namespace yuengine::audio {
struct AudioPcmStreamQueueSnapshot final {
    std::size_t queue_capacity = 0U;
    std::size_t active_queue_count = 0U;
    std::uint64_t created_queue_count = 0U;
    std::uint64_t queried_queue_count = 0U;
    std::uint64_t drained_descriptor_count = 0U;
    std::uint64_t drained_frame_count = 0U;
    std::uint64_t released_queue_count = 0U;
    std::uint64_t rejected_queue_count = 0U;
    std::uint64_t duplicate_queue_rejected_count = 0U;
    std::uint64_t stale_queue_rejected_count = 0U;
    std::uint64_t packet_rejected_count = 0U;
    std::uint64_t range_rejected_count = 0U;
    std::uint64_t sample_count_rejected_count = 0U;
    std::uint64_t byte_count_rejected_count = 0U;
    std::uint64_t chunk_rejected_count = 0U;
    std::uint64_t capacity_rejected_count = 0U;
    std::uint64_t output_capacity_rejected_count = 0U;
    std::size_t last_required_queue_count = 0U;
    std::size_t last_required_output_chunk_count = 0U;
    std::uint32_t last_failed_queue_id = 0U;
    std::uint32_t last_failed_packet_slot = 0U;
    std::uint32_t last_failed_packet_generation = 0U;
    std::uint32_t last_failed_packet_id = 0U;
    std::size_t last_failed_queue_capacity = 0U;
    std::size_t last_failed_active_queue_count = 0U;
    std::uint32_t last_queue_id = 0U;
    std::uint32_t last_packet_id = 0U;
    std::size_t last_frame_count = 0U;
    AudioPcmStreamQueueStatus last_status = AudioPcmStreamQueueStatus::NotInitialized;
    AudioPcmStreamQueueOperation last_operation = AudioPcmStreamQueueOperation::None;
};
}
