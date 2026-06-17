/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodeResultBudgetDesc.h
 * @brief Resource module decode result budget value contract.
 */
#pragma once

#include "YuEngine/Resource/ResourceConstants.h"

#include <cstdint>

namespace yuengine::resource {

/**
 * @brief Describes the decoded-byte budget for committed decode results.
 */
struct ResourceDecodeResultBudgetDesc {
    std::uint32_t decoded_byte_capacity = MAX_RESOURCE_DECODE_RESULT_TOTAL_DECODED_BYTES;
};

} // namespace yuengine::resource
