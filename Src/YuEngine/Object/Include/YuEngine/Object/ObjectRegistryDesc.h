#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectConstants.h"

namespace yuengine::object {
struct ObjectRegistryDesc final {
    std::uint32_t object_capacity = MAX_OBJECT_COUNT;
    std::uint32_t type_capacity = MAX_OBJECT_TYPE_COUNT;
};
}
