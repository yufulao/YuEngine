// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyBridgeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"

namespace yuengine::world {
struct WorldSceneAssemblyBridgeDesc final {
    std::uint32_t attachment_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t binding_capacity = MAX_WORLD_OBJECT_COUNT;
};
}
