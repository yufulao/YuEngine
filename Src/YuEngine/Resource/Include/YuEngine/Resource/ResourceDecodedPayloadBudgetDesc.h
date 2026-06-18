/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodedPayloadBudgetDesc.h
 * @brief Resource module decoded payload budget value contract.
 */
#pragma once

#include "YuEngine/Resource/ResourceConstants.h"

#include <cstdint>

namespace yuengine::resource {

/**
 * @brief Describes the decoded-byte storage budget for Resource-owned decoded payloads.
 */
struct ResourceDecodedPayloadBudgetDesc {
    std::uint32_t decoded_byte_capacity = MAX_RESOURCE_DECODED_PAYLOAD_TOTAL_BYTES;
};

} // namespace yuengine::resource
