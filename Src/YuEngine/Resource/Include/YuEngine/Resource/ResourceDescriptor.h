#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceDescriptor final {
    ResourceTypeId Type;
    ResourceLogicalKey LogicalKey;
    std::uint32_t InitialReferenceCount = 0U;
};
}
