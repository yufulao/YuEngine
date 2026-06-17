// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceCachePayloadSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceCachePayloadOperation.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceHandle.h"

namespace yuengine::resource {
struct ResourceCachePayloadSnapshot final {
    std::uint32_t budget_byte_capacity = MAX_RESOURCE_CACHE_PAYLOAD_TOTAL_BYTES;
    std::uint32_t cached_byte_count = 0U;
    std::uint32_t cached_payload_count = 0U;
    std::uint32_t cache_payload_record_count = 0U;
    std::uint64_t stored_payload_count = 0U;
    std::uint64_t read_payload_count = 0U;
    std::uint64_t released_payload_count = 0U;
    std::uint64_t rejected_payload_request_count = 0U;
    std::uint64_t duplicate_payload_rejected_count = 0U;
    std::uint64_t capacity_rejected_payload_count = 0U;
    std::uint64_t budget_rejected_payload_count = 0U;
    ResourceCachePayloadOperation last_operation = ResourceCachePayloadOperation::None;
    ResourceCachePayloadStatus last_status = ResourceCachePayloadStatus::Success;
    ResourceHandle last_resource;
    std::uint64_t last_payload_id = 0U;
    std::uint32_t last_cache_slot_index = INVALID_RESOURCE_SLOT;
    std::uint32_t last_payload_byte_count = 0U;
};
}
