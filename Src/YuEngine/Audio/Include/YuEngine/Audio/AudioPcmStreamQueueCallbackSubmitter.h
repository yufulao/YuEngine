// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmStreamQueueCallbackSubmitter.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Audio/AudioStatus.h"

namespace yuengine::audio {
using AudioPcmStreamQueueSubmitS16Function = AudioStatus (*)(void *context, std::span<const std::int16_t> interleaved_samples, std::size_t frame_count);

struct AudioPcmStreamQueueCallbackSubmitter final {
    void *context = nullptr;
    AudioPcmStreamQueueSubmitS16Function submit_s16_buffer = nullptr;
    std::size_t frames_per_buffer = 0U;
    bool started = false;
};
}
