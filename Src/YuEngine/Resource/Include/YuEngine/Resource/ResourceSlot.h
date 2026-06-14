#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceSlot final {
    ResourceTypeId Type;
    ResourceLogicalKey LogicalKey;
    std::uint32_t Generation = INVALID_RESOURCE_GENERATION;
    std::uint32_t ReferenceCount = 0U;
    bool IsActive = false;
};
}
