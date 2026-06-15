// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformManifestStreamDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"

namespace yuengine::world {
struct WorldSceneObjectTransformManifestStreamDesc final {
    std::uint32_t identity_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t transform_capacity = MAX_WORLD_OBJECT_COUNT;
};
}
