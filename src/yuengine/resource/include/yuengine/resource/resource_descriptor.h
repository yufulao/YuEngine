#pragma once

#include <cstdint>

#include "yuengine/resource/resource_logical_key.h"
#include "yuengine/resource/resource_type_id.h"

namespace yuengine::resource {
struct ResourceDescriptor final {
    ResourceTypeId Type;
    ResourceLogicalKey LogicalKey;
    std::uint32_t InitialReferenceCount = 0U;
};
}
