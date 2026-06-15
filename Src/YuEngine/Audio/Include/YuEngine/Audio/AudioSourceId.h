// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioSourceId.h

#pragma once

#include <cstdint>

namespace yuengine::audio {
struct AudioSourceId final {
    std::uint32_t slot = 0U;
    std::uint32_t generation = 0U;
};
}
