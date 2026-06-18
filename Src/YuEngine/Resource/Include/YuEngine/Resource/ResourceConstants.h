// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceConstants.h

#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::resource {
constexpr std::uint32_t MAX_RESOURCE_COUNT = 32U;
constexpr std::uint32_t MAX_RESOURCE_TYPE_COUNT = 8U;
constexpr std::uint32_t MAX_DEPENDENCY_EDGE_COUNT = 64U;
constexpr std::uint32_t MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT = 32U;
constexpr std::uint32_t MAX_RESOURCE_RESIDENCY_RECORD_COUNT = 64U;
constexpr std::uint32_t MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT = 32U;
constexpr std::uint32_t MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD = 256U;
constexpr std::uint32_t MAX_RESOURCE_CACHE_PAYLOAD_TOTAL_BYTES = 1024U;
constexpr std::uint32_t MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT = 32U;
constexpr std::uint32_t MAX_RESOURCE_DECODE_PLAN_TOTAL_DECODED_BYTES = 2048U;
constexpr std::uint32_t MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT = 32U;
constexpr std::uint32_t MAX_RESOURCE_DECODE_RESULT_TOTAL_DECODED_BYTES = 2048U;
constexpr std::uint32_t MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT = 32U;
constexpr std::uint32_t MAX_RESOURCE_DECODED_PAYLOAD_BYTES_PER_RECORD = 256U;
constexpr std::uint32_t MAX_RESOURCE_DECODED_PAYLOAD_TOTAL_BYTES = 2048U;
constexpr std::uint32_t RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT = 20U;
constexpr std::uint32_t RESOURCE_DECODE_PLAN_HEADER_VERSION = 1U;
constexpr std::uint8_t RESOURCE_DECODE_PLAN_HEADER_MAGIC_0 = 0x59U;
constexpr std::uint8_t RESOURCE_DECODE_PLAN_HEADER_MAGIC_1 = 0x52U;
constexpr std::uint8_t RESOURCE_DECODE_PLAN_HEADER_MAGIC_2 = 0x44U;
constexpr std::uint8_t RESOURCE_DECODE_PLAN_HEADER_MAGIC_3 = 0x50U;
constexpr std::size_t MAX_LOGICAL_KEY_BYTES = 64U;
constexpr std::uint32_t INVALID_RESOURCE_GENERATION = 0U;
constexpr std::uint32_t INVALID_RESOURCE_SLOT = UINT32_MAX;
}
