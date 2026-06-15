// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldObjectIdentityBridgeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"

namespace yuengine::world {
struct WorldObjectIdentityBridgeDesc final {
    std::uint32_t bridge_capacity = MAX_WORLD_OBJECT_COUNT;
};
}
