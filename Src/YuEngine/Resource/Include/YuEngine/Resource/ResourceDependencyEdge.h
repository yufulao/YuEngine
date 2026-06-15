// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDependencyEdge.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"

namespace yuengine::resource {
struct ResourceDependencyEdge final {
    std::uint32_t dependent_slot = INVALID_RESOURCE_SLOT;
    std::uint32_t dependency_slot = INVALID_RESOURCE_SLOT;
    bool is_active = false;
};
}
