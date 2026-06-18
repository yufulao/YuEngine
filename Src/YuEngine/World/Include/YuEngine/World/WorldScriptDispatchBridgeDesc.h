// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldScriptDispatchBridgeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"

namespace yuengine::world {
struct WorldScriptDispatchBridgeDesc final {
    std::uint32_t binding_capacity = WORLD_UPDATE_PHASE_COUNT;
};
}
