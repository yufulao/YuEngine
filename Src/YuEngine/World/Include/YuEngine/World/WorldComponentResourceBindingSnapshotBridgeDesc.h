// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingSnapshotBridgeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"

namespace yuengine::world {
struct WorldComponentResourceBindingSnapshotBridgeDesc final {
    std::uint32_t binding_capacity = MAX_WORLD_OBJECT_COUNT;
};
}
