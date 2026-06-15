// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceHandle.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"

namespace yuengine::resource {
struct ResourceHandle final {
    std::uint32_t slot = INVALID_RESOURCE_SLOT;
    std::uint32_t generation = INVALID_RESOURCE_GENERATION;

    /**
     * @comment Checks whether the value is valid.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsValid() const {
        if (slot == INVALID_RESOURCE_SLOT) {
            return false;
        }

        return generation != INVALID_RESOURCE_GENERATION;
    }
};
}
