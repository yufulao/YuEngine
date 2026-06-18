// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioCallbackSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioStatus.h"

namespace yuengine::audio {
struct AudioCallbackCompletion final {
    AudioStatus status = AudioStatus::Success;
    std::uint64_t sequence = 0U;
    std::size_t buffer_slot = 0U;
    std::size_t frame_count = 0U;
};

struct AudioCallbackSnapshot final {
    std::size_t buffer_capacity = 0U;
    std::size_t frames_per_buffer = 0U;
    std::uint32_t sample_rate = 0U;
    std::uint16_t channel_count = 0U;
    std::uint64_t setup_allocation_count = 0U;
    std::uint64_t submitted_buffer_count = 0U;
    std::uint64_t completed_callback_count = 0U;
    std::uint64_t failed_submission_count = 0U;
    std::uint64_t failed_callback_count = 0U;
    std::uint64_t underrun_count = 0U;
    std::uint64_t shutdown_callback_count = 0U;
    std::size_t queued_buffer_count = 0U;
    std::size_t max_queued_buffer_count = 0U;
    std::size_t drained_completion_count = 0U;
    AudioStatus last_status = AudioStatus::NotInitialized;
    bool initialized = false;
    bool started = false;
    bool shutdown = false;
};
}
