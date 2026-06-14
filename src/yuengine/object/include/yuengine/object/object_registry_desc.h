#pragma once

#include <cstdint>

#include "yuengine/object/object_constants.h"

namespace yuengine::object {
struct object_registry_desc_t final {
    std::uint32_t ObjectCapacity = MAX_OBJECT_COUNT;
    std::uint32_t TypeCapacity = MAX_OBJECT_TYPE_COUNT;
};
}
