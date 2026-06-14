#pragma once

#include <cstdint>

#include "yuengine/resource/resource_constants.h"
#include "yuengine/resource/resource_logical_key.h"
#include "yuengine/resource/resource_type_id.h"

namespace yuengine::resource {
struct ResourceSlot final {
    ResourceTypeId Type;
    ResourceLogicalKey LogicalKey;
    std::uint32_t Generation = INVALID_RESOURCE_GENERATION;
    std::uint32_t ReferenceCount = 0U;
    bool IsActive = false;
};
}
