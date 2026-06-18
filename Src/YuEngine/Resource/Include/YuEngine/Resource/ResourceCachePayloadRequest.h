// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceCachePayloadRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceCachePayloadRequest final {
    ResourceHandle resource;
    ResourceTypeId expected_type;
    std::uint64_t payload_id = 0U;
    const std::uint8_t *payload_bytes = nullptr;
    std::uint32_t payload_byte_count = 0U;
};
}
