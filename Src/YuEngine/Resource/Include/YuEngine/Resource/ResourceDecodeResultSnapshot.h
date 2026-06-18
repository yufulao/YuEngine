/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodeResultSnapshot.h
 * @brief Resource 模块解码结果快照值契约。
 */
#pragma once

#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceDecodeResultOperation.h"
#include "YuEngine/Resource/ResourceDecodeResultStatus.h"
#include "YuEngine/Resource/ResourceHandle.h"

#include <cstdint>

namespace yuengine::resource {

/**
 * @brief 汇总解码结果元数据操作和当前预算使用。
 */
struct ResourceDecodeResultSnapshot {
    std::uint32_t budget_decoded_byte_capacity = MAX_RESOURCE_DECODE_RESULT_TOTAL_DECODED_BYTES;
    std::uint32_t committed_decoded_byte_count = 0U;
    std::uint32_t active_result_count = 0U;
    std::uint32_t decode_result_record_count = 0U;
    std::uint32_t committed_result_count = 0U;
    std::uint32_t queried_result_count = 0U;
    std::uint32_t released_result_count = 0U;
    std::uint32_t rejected_result_request_count = 0U;
    std::uint32_t duplicate_result_rejected_count = 0U;
    std::uint32_t capacity_rejected_result_count = 0U;
    std::uint32_t budget_rejected_result_count = 0U;
    ResourceDecodeResultOperation last_operation = ResourceDecodeResultOperation::None;
    ResourceDecodeResultStatus last_status = ResourceDecodeResultStatus::Success;
    ResourceHandle last_resource;
    std::uint64_t last_payload_id = 0U;
    std::uint64_t last_decode_plan_id = 0U;
    std::uint64_t last_decode_result_id = 0U;
    ResourceDecodePlanAssetClass last_asset_class = ResourceDecodePlanAssetClass::Unknown;
    ResourceDecodeResultClass last_result_class = ResourceDecodeResultClass::Unknown;
    std::uint32_t last_decoded_byte_count = 0U;
    std::uint32_t last_decode_plan_slot_index = INVALID_RESOURCE_SLOT;
};

} // namespace yuengine::resource
