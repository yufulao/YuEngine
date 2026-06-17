// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmSamplePacketHandle.h

#pragma once

#include <cstdint>

namespace yuengine::audio {
struct AudioPcmSamplePacketHandle final {
    std::uint32_t slot = 0U;
    std::uint32_t generation = 0U;
};
}
