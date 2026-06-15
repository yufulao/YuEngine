// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceSlot.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceSlot final {
    ResourceTypeId type;
    ResourceLogicalKey logical_key;
    std::uint32_t generation = INVALID_RESOURCE_GENERATION;
    std::uint32_t reference_count = 0U;
    bool is_active = false;
};
}
