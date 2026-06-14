#pragma once

#include <cstdint>

#include "yuengine/object/object_constants.h"

namespace yuengine::object {
struct object_handle_t final {
    std::uint32_t Slot = INVALID_OBJECT_SLOT;
    std::uint32_t Generation = INVALID_OBJECT_GENERATION;

    bool IsValid() const {
        if (Slot == INVALID_OBJECT_SLOT) {
            return false;
        }

        return Generation != INVALID_OBJECT_GENERATION;
    }
};
}
