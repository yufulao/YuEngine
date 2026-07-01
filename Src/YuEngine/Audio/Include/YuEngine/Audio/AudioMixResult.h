// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioMixResult.h

#pragma once

#include <cstddef>

#include "YuEngine/Audio/AudioStatus.h"

namespace yuengine::audio {
struct AudioMixResult final {
    AudioStatus status = AudioStatus::InvalidDescriptor;
    std::size_t frames_written = 0U;
    std::size_t failed_mix_requested_frame_count = 0U;
    std::size_t failed_mix_output_sample_capacity = 0U;
    std::size_t failed_mix_required_sample_count = 0U;
    std::size_t failed_mix_active_voice_count = 0U;
};
}
