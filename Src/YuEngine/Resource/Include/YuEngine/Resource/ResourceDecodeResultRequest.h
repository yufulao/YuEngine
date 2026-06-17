/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodeResultRequest.h
 * @brief Resource module decode result request value contract.
 */
#pragma once

#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

#include <cstdint>

namespace yuengine::resource {

/**
 * @brief Describes a decode-result metadata operation for one resource payload and decode plan.
 */
struct ResourceDecodeResultRequest {
    ResourceHandle resource;
    ResourceTypeId expected_type;
    std::uint64_t payload_id = 0U;
    std::uint64_t decode_plan_id = 0U;
    std::uint64_t decode_result_id = 0U;
    ResourceDecodePlanAssetClass asset_class = ResourceDecodePlanAssetClass::Unknown;
    ResourceDecodeResultClass result_class = ResourceDecodeResultClass::Unknown;
    std::uint32_t decoded_byte_count = 0U;
};

} // namespace yuengine::resource
