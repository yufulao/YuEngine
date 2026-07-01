// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioDeviceSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioAccountingStatus.h"
#include "YuEngine/Audio/AudioStatus.h"

namespace yuengine::audio {
struct AudioDeviceSnapshot final {
    std::size_t source_capacity = 0U;
    std::size_t voice_capacity = 0U;
    std::size_t source_count = 0U;
    std::size_t active_voice_count = 0U;
    std::size_t last_required_source_count = 0U;
    std::size_t last_required_voice_count = 0U;
    std::size_t last_failed_mix_requested_frame_count = 0U;
    std::size_t last_failed_mix_output_sample_capacity = 0U;
    std::size_t last_failed_mix_required_sample_count = 0U;
    std::size_t last_failed_mix_active_voice_count = 0U;
    std::size_t voice_storage_capacity_before_mix = 0U;
    std::size_t voice_storage_capacity_after_last_mix = 0U;
    std::uint64_t registered_source_count = 0U;
    std::uint64_t started_voice_count = 0U;
    std::uint64_t stopped_voice_count = 0U;
    std::uint64_t mixed_frame_count = 0U;
    std::uint64_t output_sample_write_count = 0U;
    std::uint64_t failed_operation_count = 0U;
    AudioStatus last_status = AudioStatus::NotInitialized;
    std::size_t last_frames_written = 0U;
    AudioAccountingStatus allocation_accounting_status = AudioAccountingStatus::DeferredUntilYuMemoryIntegration;
};
}
