// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodePlanBudgetDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"

namespace yuengine::resource {
struct ResourceDecodePlanBudgetDesc final {
    std::uint32_t decoded_byte_capacity = MAX_RESOURCE_DECODE_PLAN_TOTAL_DECODED_BYTES;
};
}
