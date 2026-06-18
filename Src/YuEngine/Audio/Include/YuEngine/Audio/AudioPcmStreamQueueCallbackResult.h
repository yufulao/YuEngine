// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmStreamQueueCallbackResult.h

#pragma once

#include <cstddef>

#include "YuEngine/Audio/AudioPcmStreamQueueHandle.h"
#include "YuEngine/Audio/AudioStatus.h"

namespace yuengine::audio {
struct AudioPcmStreamQueueCallbackResult final {
    AudioStatus status = AudioStatus::NotInitialized;
    AudioPcmStreamQueueHandle queue{};
    std::size_t drained_chunk_count = 0U;
    std::size_t submitted_chunk_count = 0U;
    std::size_t submitted_frame_count = 0U;
    std::size_t submitted_sample_count = 0U;
    std::size_t last_submitted_first_frame = 0U;
    bool reached_final_chunk = false;
};
}
