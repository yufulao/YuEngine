#pragma once

#include <cstdint>

#include "yuengine/object/object_constants.h"
#include "yuengine/object/object_type_id.h"

namespace yuengine::object {
struct object_slot_t final {
    object_type_id_t Type;
    std::uint32_t Generation = INVALID_OBJECT_GENERATION;
    std::uint32_t ReferenceCount = 0U;
    bool IsActive = false;
};
}
