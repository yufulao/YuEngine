// Module: YuEngine Serialize
// File: Src/YuEngine/Serialize/Include/YuEngine/Serialize/SerializeConstants.h

#pragma once

#include <cstdint>

namespace yuengine::serialize {
constexpr std::uint32_t MAX_STREAM_BYTE_COUNT = 4096U;
constexpr std::uint32_t MAX_RECORDS_PER_STREAM = 32U;
constexpr std::uint32_t MAX_FIELDS_PER_STREAM = 64U;
constexpr std::uint32_t MAX_FIELDS_PER_RECORD = 16U;
constexpr std::uint32_t MAX_FIELD_PAYLOAD_BYTE_COUNT = 256U;

constexpr std::uint32_t STREAM_MAGIC = 0x52535559U;
constexpr std::uint16_t STREAM_MAJOR_VERSION = 1U;
constexpr std::uint16_t STREAM_MINOR_VERSION = 0U;
constexpr std::uint32_t STREAM_FLAGS = 0U;

constexpr std::uint32_t STREAM_HEADER_BYTE_COUNT = 16U;
constexpr std::uint32_t RECORD_HEADER_BYTE_COUNT = 8U;
constexpr std::uint32_t FIELD_HEADER_BYTE_COUNT = 12U;

constexpr std::uint32_t STREAM_MAGIC_OFFSET = 0U;
constexpr std::uint32_t STREAM_MAJOR_VERSION_OFFSET = 4U;
constexpr std::uint32_t STREAM_MINOR_VERSION_OFFSET = 6U;
constexpr std::uint32_t STREAM_FLAGS_OFFSET = 8U;
constexpr std::uint32_t STREAM_RECORD_COUNT_OFFSET = 12U;

constexpr std::uint32_t UINT32_PAYLOAD_BYTE_COUNT = 4U;
constexpr std::uint32_t INT32_PAYLOAD_BYTE_COUNT = 4U;
constexpr std::uint32_t UINT64_PAYLOAD_BYTE_COUNT = 8U;
constexpr std::uint32_t INT64_PAYLOAD_BYTE_COUNT = 8U;
}
