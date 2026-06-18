// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmSamplePacketSlot.h

#pragma once

#include <cstdint>

#include "YuEngine/Audio/AudioPcmSamplePacketRecord.h"

namespace yuengine::audio {
struct AudioPcmSamplePacketSlot final {
    AudioPcmSamplePacketRecord record{};
    std::uint32_t generation = 1U;
    bool is_active = false;
};
}
