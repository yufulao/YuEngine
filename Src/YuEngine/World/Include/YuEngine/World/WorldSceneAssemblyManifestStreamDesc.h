// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyManifestStreamDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"

namespace yuengine::world {
struct WorldSceneAssemblyManifestStreamDesc final {
    std::uint32_t attachment_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t binding_capacity = MAX_WORLD_OBJECT_COUNT;
};
}
