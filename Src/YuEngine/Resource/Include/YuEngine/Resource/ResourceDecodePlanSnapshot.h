// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodePlanSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodePlanOperation.h"
#include "YuEngine/Resource/ResourceDecodePlanStatus.h"
#include "YuEngine/Resource/ResourceHandle.h"

namespace yuengine::resource {
struct ResourceDecodePlanSnapshot final {
    std::uint32_t budget_decoded_byte_capacity = MAX_RESOURCE_DECODE_PLAN_TOTAL_DECODED_BYTES;
    std::uint32_t planned_decoded_byte_count = 0U;
    std::uint32_t active_plan_count = 0U;
    std::uint32_t decode_plan_record_count = 0U;
    std::uint32_t last_required_plan_count = 0U;
    std::uint32_t last_required_decoded_byte_count = 0U;
    std::uint64_t created_plan_count = 0U;
    std::uint64_t queried_plan_count = 0U;
    std::uint64_t released_plan_count = 0U;
    std::uint64_t rejected_plan_request_count = 0U;
    std::uint64_t duplicate_plan_rejected_count = 0U;
    std::uint64_t capacity_rejected_plan_count = 0U;
    std::uint64_t budget_rejected_plan_count = 0U;
    std::uint64_t invalid_header_rejected_count = 0U;
    ResourceDecodePlanOperation last_operation = ResourceDecodePlanOperation::None;
    ResourceDecodePlanStatus last_status = ResourceDecodePlanStatus::Success;
    ResourceHandle last_resource;
    std::uint64_t last_payload_id = 0U;
    std::uint64_t last_decode_plan_id = 0U;
    ResourceDecodePlanAssetClass last_asset_class = ResourceDecodePlanAssetClass::Unknown;
    std::uint32_t last_cache_slot_index = INVALID_RESOURCE_SLOT;
    std::uint32_t last_source_byte_count = 0U;
    std::uint32_t last_expected_decoded_byte_count = 0U;
    std::uint32_t last_header_version = 0U;
};
}
