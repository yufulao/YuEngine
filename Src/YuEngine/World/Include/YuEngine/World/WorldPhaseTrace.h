// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldPhaseTrace.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldUpdatePhase.h"

namespace yuengine::world {
struct WorldPhaseTrace final {
    WorldUpdatePhase phase = WorldUpdatePhase::BeginFrame;
    std::uint64_t frame_index = 0U;
    std::uint32_t active_object_count = 0U;
    std::uint32_t skipped_object_count = 0U;
};
}
