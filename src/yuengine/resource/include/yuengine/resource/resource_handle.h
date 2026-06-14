#pragma once

#include <cstdint>

#include "yuengine/resource/resource_constants.h"

namespace yuengine::resource {
struct resource_handle_t final {
    std::uint32_t Slot = INVALID_RESOURCE_SLOT;
    std::uint32_t Generation = INVALID_RESOURCE_GENERATION;

    bool IsValid() const {
        if (Slot == INVALID_RESOURCE_SLOT) {
            return false;
        }

        return Generation != INVALID_RESOURCE_GENERATION;
    }
};
}
