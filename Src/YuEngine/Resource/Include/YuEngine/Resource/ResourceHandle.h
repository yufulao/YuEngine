#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"

namespace yuengine::resource {
struct ResourceHandle final {
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
