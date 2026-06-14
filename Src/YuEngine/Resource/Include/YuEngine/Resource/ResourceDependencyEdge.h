#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"

namespace yuengine::resource {
struct ResourceDependencyEdge final {
    std::uint32_t DependentSlot = INVALID_RESOURCE_SLOT;
    std::uint32_t DependencySlot = INVALID_RESOURCE_SLOT;
    bool IsActive = false;
};
}
