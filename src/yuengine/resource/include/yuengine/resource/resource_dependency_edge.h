#pragma once

#include <cstdint>

#include "yuengine/resource/resource_constants.h"

namespace yuengine::resource {
struct resource_dependency_edge_t final {
    std::uint32_t DependentSlot = INVALID_RESOURCE_SLOT;
    std::uint32_t DependencySlot = INVALID_RESOURCE_SLOT;
    bool IsActive = false;
};
}
