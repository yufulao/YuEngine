#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"

namespace yuengine::resource {
struct ResourceRegistryDesc final {
    std::uint32_t resource_capacity = MAX_RESOURCE_COUNT;
    std::uint32_t type_capacity = MAX_RESOURCE_TYPE_COUNT;
    std::uint32_t dependency_edge_capacity = MAX_DEPENDENCY_EDGE_COUNT;
};
}
