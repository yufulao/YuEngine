// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmStreamQueueHandle.h

#pragma once

#include <cstdint>

namespace yuengine::audio {
struct AudioPcmStreamQueueHandle final {
    std::uint32_t slot = 0U;
    std::uint32_t generation = 0U;
};
}
