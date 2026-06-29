// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceCachePayloadBridgeSnapshot.h
#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Streaming/PackageResourceStagingStatus.h"
#include "YuEngine/Streaming/ResourceCachePayloadBridgeStatus.h"

namespace yuengine::streaming {

struct ResourceCachePayloadBridgeSnapshot final {
    std::uint64_t submitted_count = 0U;
    std::uint64_t completed_count = 0U;
    std::uint64_t failed_count = 0U;
    std::uint64_t rejected_count = 0U;
    ResourceCachePayloadBridgeStatus last_status = ResourceCachePayloadBridgeStatus::Success;
    resource::ResourceCachePayloadStatus last_cache_payload_status = resource::ResourceCachePayloadStatus::Success;
    PackageResourceStagingStatus last_staging_status = PackageResourceStagingStatus::Success;
    std::uint64_t last_payload_id = 0U;
    std::uint64_t last_payload_logical_byte_count = 0U;
    std::uint64_t last_payload_window_byte_offset = 0U;
    std::uint64_t last_payload_window_byte_size = 0U;
    std::uint32_t last_payload_byte_count = 0U;
    memory::MemoryAccountingStatus allocation_accounting_status = memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
};

} // namespace yuengine::streaming
