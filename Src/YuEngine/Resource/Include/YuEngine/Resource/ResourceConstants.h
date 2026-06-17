// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceConstants.h

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
constexpr std::size_t MAX_LOGICAL_KEY_BYTES = 64U;
constexpr std::uint32_t INVALID_RESOURCE_GENERATION = 0U;
constexpr std::uint32_t INVALID_RESOURCE_SLOT = UINT32_MAX;
}
