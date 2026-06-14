#pragma once

#include <cstdint>

#include "yuengine/object/ObjectConstants.h"

namespace yuengine::object {
struct ObjectRegistryDesc final {
    std::uint32_t ObjectCapacity = MAX_OBJECT_COUNT;
    std::uint32_t TypeCapacity = MAX_OBJECT_TYPE_COUNT;
};
}
