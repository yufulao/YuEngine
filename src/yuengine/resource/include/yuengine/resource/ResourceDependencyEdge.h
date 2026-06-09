#pragma once

#include <cstdint>

#include "yuengine/resource/ResourceConstants.h"

namespace yuengine::resource
{
struct ResourceDependencyEdge final
{
    std::uint32_t DependentSlot = INVALID_RESOURCE_SLOT;
    std::uint32_t DependencySlot = INVALID_RESOURCE_SLOT;
    bool IsActive = false;
};
}
