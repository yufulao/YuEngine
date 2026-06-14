#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceDescriptor final {
    ResourceTypeId type;
    ResourceLogicalKey logical_key;
    std::uint32_t initial_reference_count = 0U;
};
}
