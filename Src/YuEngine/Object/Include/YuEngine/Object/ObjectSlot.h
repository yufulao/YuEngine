#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectConstants.h"
#include "YuEngine/Object/ObjectTypeId.h"

namespace yuengine::object {
struct ObjectSlot final {
    ObjectTypeId Type;
    std::uint32_t Generation = INVALID_OBJECT_GENERATION;
    std::uint32_t ReferenceCount = 0U;
    bool IsActive = false;
};
}
