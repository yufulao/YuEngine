#pragma once

#include <cstdint>

#include "yuengine/resource/resource_constants.h"

namespace yuengine::resource {
struct resource_registry_desc_t final {
    std::uint32_t ResourceCapacity = MAX_RESOURCE_COUNT;
    std::uint32_t TypeCapacity = MAX_RESOURCE_TYPE_COUNT;
    std::uint32_t DependencyEdgeCapacity = MAX_DEPENDENCY_EDGE_COUNT;
};
}
