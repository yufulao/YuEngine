// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmStreamQueueSlot.h

#pragma once

#include <cstdint>

#include "YuEngine/Audio/AudioPcmStreamQueueRecord.h"

namespace yuengine::audio {
struct AudioPcmStreamQueueSlot final {
    AudioPcmStreamQueueRecord record{};
    std::uint32_t generation = 1U;
    bool is_active = false;
};
}
