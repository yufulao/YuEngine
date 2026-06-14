#pragma once

#include <cstdint>

#include "yuengine/resource/resource_logical_key.h"
#include "yuengine/resource/resource_type_id.h"

namespace yuengine::resource {
struct resource_descriptor_t final {
    resource_type_id_t Type;
    ResourceLogicalKey LogicalKey;
    std::uint32_t InitialReferenceCount = 0U;
};
}
