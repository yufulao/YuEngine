// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodePlanRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodePlanOperation.h"
#include "YuEngine/Resource/ResourceDecodePlanStatus.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceDecodePlanRecord final {
    ResourceDecodePlanOperation operation = ResourceDecodePlanOperation::None;
    ResourceHandle resource;
    ResourceTypeId expected_type;
    std::uint64_t payload_id = 0U;
    std::uint64_t decode_plan_id = 0U;
    ResourceDecodePlanAssetClass asset_class = ResourceDecodePlanAssetClass::Unknown;
    std::uint32_t source_byte_count = 0U;
    std::uint32_t expected_decoded_byte_count = 0U;
    std::uint32_t header_version = 0U;
    std::uint32_t cache_slot_index = INVALID_RESOURCE_SLOT;
    ResourceDecodePlanStatus status = ResourceDecodePlanStatus::Success;
    bool is_active = false;
};
}
