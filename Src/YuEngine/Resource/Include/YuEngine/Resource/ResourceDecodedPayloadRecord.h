/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodedPayloadRecord.h
 * @brief Resource module decoded payload record value contract.
 */
#pragma once

#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodedPayloadOperation.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

#include <cstdint>

namespace yuengine::resource {

/**
 * @brief Stores Resource-owned decoded payload metadata for one decode result.
 */
struct ResourceDecodedPayloadRecord {
    ResourceDecodedPayloadOperation operation = ResourceDecodedPayloadOperation::None;
    ResourceHandle resource;
    ResourceTypeId expected_type;
    std::uint64_t payload_id = 0U;
    std::uint64_t decode_plan_id = 0U;
    std::uint64_t decode_result_id = 0U;
    std::uint64_t decoded_payload_id = 0U;
    ResourceDecodePlanAssetClass asset_class = ResourceDecodePlanAssetClass::Unknown;
    ResourceDecodeResultClass result_class = ResourceDecodeResultClass::Unknown;
    std::uint32_t decoded_byte_count = 0U;
    std::uint32_t decode_result_slot_index = INVALID_RESOURCE_SLOT;
    std::uint32_t decoded_payload_slot_index = INVALID_RESOURCE_SLOT;
    ResourceDecodedPayloadStatus status = ResourceDecodedPayloadStatus::Success;
    bool is_active = false;
};

} // namespace yuengine::resource
