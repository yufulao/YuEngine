// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Src/ResourceCachePayloadBridge.cpp
#include "YuEngine/Streaming/ResourceCachePayloadBridge.h"

#include <limits>

#include "YuEngine/Resource/ResourceCachePayloadRequest.h"
#include "YuEngine/Resource/ResourceRegistry.h"

namespace yuengine::streaming {
namespace {

bool PayloadWindowEndOverflows(std::uint64_t byte_offset, std::uint64_t byte_size) {
    const std::uint64_t max_value = std::numeric_limits<std::uint64_t>::max();
    const std::uint64_t remaining_byte_count = max_value - byte_offset;
    return byte_size > remaining_byte_count;
}

bool IsPayloadWindowOutOfBounds(std::uint64_t logical_byte_count, std::uint64_t byte_offset, std::uint64_t byte_size) {
    if (byte_size == 0U) {
        return true;
    }
    if (logical_byte_count == 0U) {
        return true;
    }
    if (PayloadWindowEndOverflows(byte_offset, byte_size)) {
        return true;
    }

    const std::uint64_t window_end = byte_offset + byte_size;
    return window_end > logical_byte_count;
}

bool StagedByteRangeExceedsBuffer(const ResourceCachePayloadBridgeRequest &request) {
    const std::uint64_t staged_byte_offset = request.staging_completion.staged_byte_offset;
    const std::uint64_t staged_byte_count = request.staging_completion.staged_byte_count;
    if (PayloadWindowEndOverflows(staged_byte_offset, staged_byte_count)) {
        return true;
    }

    const std::uint64_t staged_byte_end = staged_byte_offset + staged_byte_count;
    const std::uint64_t buffer_byte_count = static_cast<std::uint64_t>(request.staged_bytes.size());
    return staged_byte_end > buffer_byte_count;
}

} // namespace

ResourceCachePayloadBridge::ResourceCachePayloadBridge() = default;

ResourceCachePayloadBridgeResult ResourceCachePayloadBridge::StorePayload(const ResourceCachePayloadBridgeRequest &request) {
    ResourceCachePayloadBridgeResult result = BuildResult(request);
    result.status = ValidateRequest(request);
    if (result.status != ResourceCachePayloadBridgeStatus::Success) {
        RecordRejected(result);
        return result;
    }

    ++snapshot_.submitted_count;

    const std::size_t staged_byte_offset = static_cast<std::size_t>(request.staging_completion.staged_byte_offset);
    const std::size_t staged_byte_count = static_cast<std::size_t>(request.staging_completion.staged_byte_count);
    const std::span<const std::uint8_t> payload_bytes = request.staged_bytes.subspan(staged_byte_offset, staged_byte_count);

    resource::ResourceCachePayloadRequest payload_request;
    payload_request.resource = request.staging_completion.resource;
    payload_request.expected_type = request.staging_completion.expected_type;
    payload_request.payload_id = request.payload_id;
    payload_request.payload_logical_byte_count = request.payload_logical_byte_count;
    payload_request.payload_window_byte_offset = request.payload_window_byte_offset;
    payload_request.payload_window_byte_size = request.payload_window_byte_size;
    payload_request.payload_bytes = payload_bytes.data();
    payload_request.payload_byte_count = static_cast<std::uint32_t>(payload_bytes.size());

    result.cache_payload_status = request.resource_registry->StoreCachePayload(payload_request);
    if (result.cache_payload_status != resource::ResourceCachePayloadStatus::Success) {
        result.status = ResourceCachePayloadBridgeStatus::ResourceStoreFailed;
        RecordFailed(result);
        return result;
    }

    result.status = ResourceCachePayloadBridgeStatus::Success;
    RecordCompleted(result);
    return result;
}

ResourceCachePayloadBridgeSnapshot ResourceCachePayloadBridge::Snapshot() const {
    return snapshot_;
}

ResourceCachePayloadBridgeStatus ResourceCachePayloadBridge::ValidateRequest(const ResourceCachePayloadBridgeRequest &request) const {
    if (request.resource_registry == nullptr) {
        return ResourceCachePayloadBridgeStatus::InvalidArgument;
    }
    if (request.payload_id == 0U) {
        return ResourceCachePayloadBridgeStatus::InvalidArgument;
    }
    if (request.staging_completion.status != PackageResourceStagingStatus::Success) {
        return ResourceCachePayloadBridgeStatus::StagingFailed;
    }
    if (request.staging_completion.staged_byte_count == 0U) {
        return ResourceCachePayloadBridgeStatus::StagingByteCountMismatch;
    }
    if (request.payload_window_byte_size != request.staging_completion.staged_byte_count) {
        return ResourceCachePayloadBridgeStatus::StagingByteCountMismatch;
    }
    const std::size_t staged_file_byte_count = static_cast<std::size_t>(request.staging_completion.staged_byte_count);
    if (request.staging_completion.file_byte_count != staged_file_byte_count) {
        return ResourceCachePayloadBridgeStatus::StagingByteCountMismatch;
    }
    if (IsPayloadWindowOutOfBounds(
            request.payload_logical_byte_count,
            request.payload_window_byte_offset,
            request.payload_window_byte_size)) {
        return ResourceCachePayloadBridgeStatus::PayloadWindowOutOfBounds;
    }
    if (StagedByteRangeExceedsBuffer(request)) {
        return ResourceCachePayloadBridgeStatus::StagingByteRangeOutOfBounds;
    }

    return ResourceCachePayloadBridgeStatus::Success;
}

ResourceCachePayloadBridgeResult ResourceCachePayloadBridge::BuildResult(const ResourceCachePayloadBridgeRequest &request) const {
    ResourceCachePayloadBridgeResult result;
    result.status = ResourceCachePayloadBridgeStatus::InvalidArgument;
    result.cache_payload_status = resource::ResourceCachePayloadStatus::Success;
    result.staging_status = request.staging_completion.status;
    result.payload_id = request.payload_id;
    result.payload_logical_byte_count = request.payload_logical_byte_count;
    result.payload_window_byte_offset = request.payload_window_byte_offset;
    result.payload_window_byte_size = request.payload_window_byte_size;
    result.payload_byte_count = request.staging_completion.staged_byte_count;
    return result;
}

void ResourceCachePayloadBridge::RecordRejected(const ResourceCachePayloadBridgeResult &result) {
    ++snapshot_.rejected_count;
    StoreLastResult(result);
}

void ResourceCachePayloadBridge::RecordFailed(const ResourceCachePayloadBridgeResult &result) {
    ++snapshot_.failed_count;
    StoreLastResult(result);
}

void ResourceCachePayloadBridge::RecordCompleted(const ResourceCachePayloadBridgeResult &result) {
    ++snapshot_.completed_count;
    StoreLastResult(result);
}

void ResourceCachePayloadBridge::StoreLastResult(const ResourceCachePayloadBridgeResult &result) {
    snapshot_.last_status = result.status;
    snapshot_.last_cache_payload_status = result.cache_payload_status;
    snapshot_.last_staging_status = result.staging_status;
    snapshot_.last_payload_id = result.payload_id;
    snapshot_.last_payload_logical_byte_count = result.payload_logical_byte_count;
    snapshot_.last_payload_window_byte_offset = result.payload_window_byte_offset;
    snapshot_.last_payload_window_byte_size = result.payload_window_byte_size;
    snapshot_.last_payload_byte_count = result.payload_byte_count;
}

} // namespace yuengine::streaming
