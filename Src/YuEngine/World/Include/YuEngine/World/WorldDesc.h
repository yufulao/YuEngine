// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"

namespace yuengine::world {
struct WorldDesc final {
    std::uint32_t object_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t phase_trace_capacity = MAX_WORLD_PHASE_TRACE_COUNT;
};
}
