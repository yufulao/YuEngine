#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"

namespace yuengine::resource {
struct ResourceRegistryDesc final {
    std::uint32_t ResourceCapacity = MAX_RESOURCE_COUNT;
    std::uint32_t TypeCapacity = MAX_RESOURCE_TYPE_COUNT;
    std::uint32_t DependencyEdgeCapacity = MAX_DEPENDENCY_EDGE_COUNT;
};
}
