/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodeResultBudgetDesc.h
 * @brief Resource 模块 decode 结果 budget 值契约。
 */
#pragma once

#include "YuEngine/Resource/ResourceConstants.h"

#include <cstdint>

namespace yuengine::resource {

/**
 * @brief 描述用于已提交 decode 结果的 decoded-byte budget。
 */
struct ResourceDecodeResultBudgetDesc {
    std::uint32_t decoded_byte_capacity = MAX_RESOURCE_DECODE_RESULT_TOTAL_DECODED_BYTES;
};

} // namespace yuengine::resource
