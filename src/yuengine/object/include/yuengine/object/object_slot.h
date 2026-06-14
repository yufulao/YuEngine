#pragma once

#include <cstdint>

#include "yuengine/object/ObjectConstants.h"
#include "yuengine/object/ObjectTypeId.h"

namespace yuengine::object {
struct ObjectSlot final {
    ObjectTypeId Type;
    std::uint32_t Generation = INVALID_OBJECT_GENERATION;
    std::uint32_t ReferenceCount = 0U;
    bool IsActive = false;
};
}
