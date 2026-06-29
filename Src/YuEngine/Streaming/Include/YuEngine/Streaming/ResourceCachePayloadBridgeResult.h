// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceCachePayloadBridgeResult.h
#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Streaming/PackageResourceStagingStatus.h"
#include "YuEngine/Streaming/ResourceCachePayloadBridgeStatus.h"

namespace yuengine::streaming {

struct ResourceCachePayloadBridgeResult final {
    ResourceCachePayloadBridgeStatus status = ResourceCachePayloadBridgeStatus::InvalidArgument;
    resource::ResourceCachePayloadStatus cache_payload_status = resource::ResourceCachePayloadStatus::Success;
    PackageResourceStagingStatus staging_status = PackageResourceStagingStatus::Success;
    std::uint64_t payload_id = 0U;
    std::uint64_t payload_logical_byte_count = 0U;
    std::uint64_t payload_window_byte_offset = 0U;
    std::uint64_t payload_window_byte_size = 0U;
    std::uint32_t payload_byte_count = 0U;
};

} // namespace yuengine::streaming
