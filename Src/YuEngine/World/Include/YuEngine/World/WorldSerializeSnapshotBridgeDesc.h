// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSerializeSnapshotBridgeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldSerializeSnapshotConstants.h"

namespace yuengine::world {
struct WorldSerializeSnapshotBridgeDesc final {
    std::uint32_t phase_trace_capacity = MAX_WORLD_SERIALIZE_PHASE_TRACE_COUNT;
};
}
