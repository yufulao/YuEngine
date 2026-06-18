// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceCachePayloadBudgetDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"

namespace yuengine::resource {
struct ResourceCachePayloadBudgetDesc final {
    std::uint32_t byte_capacity = MAX_RESOURCE_CACHE_PAYLOAD_TOTAL_BYTES;
};
}
