/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodedPayloadSnapshot.h
 * @brief Resource 模块解码载荷快照值契约。
 */
#pragma once

#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodedPayloadOperation.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceHandle.h"

#include <cstdint>

namespace yuengine::resource {

/**
 * @brief 汇总解码载荷存储操作和当前预算使用。
 */
struct ResourceDecodedPayloadSnapshot {
    std::uint32_t budget_decoded_byte_capacity = MAX_RESOURCE_DECODED_PAYLOAD_TOTAL_BYTES;
    std::uint32_t budget_payload_reference_capacity = MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT;
    std::uint32_t stored_decoded_byte_count = 0U;
    std::uint32_t active_payload_count = 0U;
    std::uint32_t decoded_payload_record_count = 0U;
    std::uint32_t stored_payload_count = 0U;
    std::uint32_t queried_payload_count = 0U;
    std::uint32_t read_payload_count = 0U;
    std::uint32_t released_payload_count = 0U;
    std::uint32_t dependent_cleared_payload_count = 0U;
    std::uint32_t rejected_payload_request_count = 0U;
    std::uint32_t duplicate_payload_rejected_count = 0U;
    std::uint32_t capacity_rejected_payload_count = 0U;
    std::uint32_t budget_rejected_payload_count = 0U;
    std::uint32_t output_buffer_too_small_count = 0U;
    std::uint32_t reference_budget_rejected_payload_count = 0U;
    std::uint32_t payload_window_rejected_count = 0U;
    ResourceDecodedPayloadOperation last_operation = ResourceDecodedPayloadOperation::None;
    ResourceDecodedPayloadStatus last_status = ResourceDecodedPayloadStatus::Success;
    ResourceHandle last_resource;
    std::uint64_t last_payload_id = 0U;
    std::uint64_t last_decode_plan_id = 0U;
    std::uint64_t last_decode_result_id = 0U;
    std::uint64_t last_decoded_payload_id = 0U;
    std::uint64_t last_payload_window_byte_offset = 0U;
    std::uint64_t last_payload_window_byte_size = 0U;
    ResourceDecodePlanAssetClass last_asset_class = ResourceDecodePlanAssetClass::Unknown;
    ResourceDecodeResultClass last_result_class = ResourceDecodeResultClass::Unknown;
    std::uint32_t last_decoded_byte_count = 0U;
    std::uint32_t last_decode_result_slot_index = INVALID_RESOURCE_SLOT;
    std::uint32_t last_decoded_payload_slot_index = INVALID_RESOURCE_SLOT;
};

} // namespace yuengine::resource
