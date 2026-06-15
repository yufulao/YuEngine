// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldObjectId.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"

namespace yuengine::world {
struct WorldObjectId final {
    std::uint32_t value = INVALID_WORLD_OBJECT_ID_VALUE;

    /**
     * @comment Checks whether the world object id is valid.
     * @return True when the id is valid; false otherwise.
     */
    bool IsValid() const {
        return value != INVALID_WORLD_OBJECT_ID_VALUE;
    }
};
}
