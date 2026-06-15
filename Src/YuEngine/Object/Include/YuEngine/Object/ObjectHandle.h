// Module: YuEngine Object
// File: Src/YuEngine/Object/Include/YuEngine/Object/ObjectHandle.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectConstants.h"

namespace yuengine::object {
struct ObjectHandle final {
    std::uint32_t slot = INVALID_OBJECT_SLOT;
    std::uint32_t generation = INVALID_OBJECT_GENERATION;

    /**
     * @comment Checks whether the value is valid.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsValid() const {
        if (slot == INVALID_OBJECT_SLOT) {
            return false;
        }

        return generation != INVALID_OBJECT_GENERATION;
    }
};
}
