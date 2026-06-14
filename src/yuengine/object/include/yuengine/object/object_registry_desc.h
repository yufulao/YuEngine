#pragma once

#include <cstdint>

#include "yuengine/object/object_constants.h"

namespace yuengine::object {
struct ObjectRegistryDesc final {
    std::uint32_t ObjectCapacity = MAX_OBJECT_COUNT;
    std::uint32_t TypeCapacity = MAX_OBJECT_TYPE_COUNT;
};
}
