// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceCachePayloadRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceCachePayloadOperation.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceCachePayloadRecord final {
    ResourceCachePayloadOperation operation = ResourceCachePayloadOperation::None;
    ResourceHandle resource;
    ResourceTypeId expected_type;
    std::uint64_t payload_id = 0U;
    std::uint32_t payload_byte_count = 0U;
    std::uint32_t cache_slot_index = INVALID_RESOURCE_SLOT;
    ResourceCachePayloadStatus status = ResourceCachePayloadStatus::Success;
    bool is_active = false;
};
}
