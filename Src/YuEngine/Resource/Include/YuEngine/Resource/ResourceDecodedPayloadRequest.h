/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodedPayloadRequest.h
 * @brief Resource 模块解码载荷请求值契约。
 */
#pragma once

#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

#include <cstdint>

namespace yuengine::resource {

/**
 * @brief 描述基于 import-ready decode 结果的 decoded payload 操作。
 */
struct ResourceDecodedPayloadRequest {
    ResourceHandle resource;
    ResourceTypeId expected_type;
    std::uint64_t payload_id = 0U;
    std::uint64_t decode_plan_id = 0U;
    std::uint64_t decode_result_id = 0U;
    std::uint64_t decoded_payload_id = 0U;
    ResourceDecodePlanAssetClass asset_class = ResourceDecodePlanAssetClass::Unknown;
    ResourceDecodeResultClass result_class = ResourceDecodeResultClass::Unknown;
    const std::uint8_t *decoded_bytes = nullptr;
    std::uint32_t decoded_byte_count = 0U;
};

} // namespace yuengine::resource
