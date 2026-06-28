/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodedPayloadBudgetDesc.h
 * @brief Resource 模块解码载荷预算值契约。
 */
#pragma once

#include "YuEngine/Resource/ResourceConstants.h"

#include <cstdint>

namespace yuengine::resource {

/**
 * @brief 描述 decoded-字节 存储 budget 用于 Resource 自有 decoded payloads.
 */
struct ResourceDecodedPayloadBudgetDesc {
    std::uint32_t decoded_byte_capacity = MAX_RESOURCE_DECODED_PAYLOAD_TOTAL_BYTES;
    std::uint32_t payload_reference_capacity = MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT;
};

} // namespace yuengine::resource
