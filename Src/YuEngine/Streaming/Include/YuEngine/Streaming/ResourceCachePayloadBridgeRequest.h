// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceCachePayloadBridgeRequest.h
#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Streaming/PackageResourceStagingCompletion.h"

namespace yuengine::resource {

class ResourceRegistry;

} // namespace yuengine::resource

namespace yuengine::streaming {

struct ResourceCachePayloadBridgeRequest final {
    resource::ResourceRegistry *resource_registry = nullptr;
    PackageResourceStagingCompletion staging_completion;
    std::span<const std::uint8_t> staged_bytes;
    std::uint64_t payload_id = 0U;
    std::uint64_t payload_logical_byte_count = 0U;
    std::uint64_t payload_window_byte_offset = 0U;
    std::uint64_t payload_window_byte_size = 0U;
};

} // namespace yuengine::streaming
