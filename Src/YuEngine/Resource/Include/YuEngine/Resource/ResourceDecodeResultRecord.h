/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodeResultRecord.h
 * @brief Resource module decode result record value contract.
 */
#pragma once

#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceDecodeResultOperation.h"
#include "YuEngine/Resource/ResourceDecodeResultStatus.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

#include <cstdint>

namespace yuengine::resource {

/**
 * @brief Stores import-ready decode-result metadata for one resource payload and decode plan.
 */
struct ResourceDecodeResultRecord {
    ResourceDecodeResultOperation operation = ResourceDecodeResultOperation::None;
    ResourceHandle resource;
    ResourceTypeId expected_type;
    std::uint64_t payload_id = 0U;
    std::uint64_t decode_plan_id = 0U;
    std::uint64_t decode_result_id = 0U;
    ResourceDecodePlanAssetClass asset_class = ResourceDecodePlanAssetClass::Unknown;
    ResourceDecodeResultClass result_class = ResourceDecodeResultClass::Unknown;
    std::uint32_t decoded_byte_count = 0U;
    std::uint32_t decode_plan_slot_index = INVALID_RESOURCE_SLOT;
    ResourceDecodeResultStatus status = ResourceDecodeResultStatus::Success;
    bool is_active = false;
};

} // namespace yuengine::resource
