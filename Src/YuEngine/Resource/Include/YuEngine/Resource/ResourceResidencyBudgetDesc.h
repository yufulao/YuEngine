// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceResidencyBudgetDesc.h

#pragma once

#include <cstdint>

namespace yuengine::resource {
struct ResourceResidencyBudgetDesc final {
    std::uint32_t byte_capacity = 0U;
};
}
