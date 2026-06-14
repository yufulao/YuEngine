#pragma once

#include <cstdint>

#include "yuengine/resource/ResourceConstants.h"
#include "yuengine/resource/ResourceLogicalKey.h"
#include "yuengine/resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceSlot final {
    ResourceTypeId Type;
    ResourceLogicalKey LogicalKey;
    std::uint32_t Generation = INVALID_RESOURCE_GENERATION;
    std::uint32_t ReferenceCount = 0U;
    bool IsActive = false;
};
}
