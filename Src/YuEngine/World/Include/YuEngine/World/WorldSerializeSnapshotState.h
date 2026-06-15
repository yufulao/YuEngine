// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSerializeSnapshotState.h

#pragma once

#include <cstdint>

namespace yuengine::world {
struct WorldSerializeSnapshotState final {
    std::uint32_t world_snapshot_count = 0U;
    std::uint32_t phase_trace_count = 0U;
    std::uint32_t transform_snapshot_count = 0U;
    std::uint32_t committed_byte_count = 0U;
};
}
