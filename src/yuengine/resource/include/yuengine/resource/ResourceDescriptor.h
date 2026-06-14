#pragma once

#include <cstdint>

#include "yuengine/resource/ResourceLogicalKey.h"
#include "yuengine/resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceDescriptor final {
    ResourceTypeId Type;
    ResourceLogicalKey LogicalKey;
    std::uint32_t InitialReferenceCount = 0U;
};
}
