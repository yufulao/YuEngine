// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioVoiceSlot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioSourceId.h"

namespace yuengine::audio {
struct AudioVoiceSlot final {
    bool is_active = false;
    std::uint32_t generation = 1U;
    AudioSourceId source{};
    std::size_t cursor_frame = 0U;
    std::uint32_t gain_q15 = 0U;
};
}
