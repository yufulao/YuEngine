#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectConstants.h"

namespace yuengine::object {
struct ObjectHandle final {
    std::uint32_t slot = INVALID_OBJECT_SLOT;
    std::uint32_t generation = INVALID_OBJECT_GENERATION;

    bool IsValid() const {
        if (slot == INVALID_OBJECT_SLOT) {
            return false;
        }

        return generation != INVALID_OBJECT_GENERATION;
    }
};
}
