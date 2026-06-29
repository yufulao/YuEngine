// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceStreamingPipelineSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/File/AsyncFileReadStatus.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Streaming/PackageResourceStagingStatus.h"
#include "YuEngine/Streaming/ResourceCachePayloadBridgeStatus.h"
#include "YuEngine/Streaming/ResourceStreamingPipelineStatus.h"
#include "YuEngine/Streaming/ResourceUploadCommitStatus.h"
#include "YuEngine/Streaming/ResourceUploadStatus.h"

namespace yuengine::streaming {
struct ResourceStreamingPipelineSnapshot final {
    std::uint64_t submitted_count = 0U;
    std::uint64_t completed_count = 0U;
    std::uint64_t failed_count = 0U;
    std::uint64_t rejected_count = 0U;
    ResourceStreamingPipelineStatus last_status = ResourceStreamingPipelineStatus::Success;
    PackageResourceStagingStatus last_staging_status = PackageResourceStagingStatus::Success;
    file::AsyncFileReadStatus last_async_file_status = file::AsyncFileReadStatus::Success;
    ResourceCachePayloadBridgeStatus last_cache_payload_status = ResourceCachePayloadBridgeStatus::Success;
    resource::ResourceCachePayloadStatus last_cache_payload_resource_status =
        resource::ResourceCachePayloadStatus::Success;
    ResourceUploadStatus last_upload_status = ResourceUploadStatus::Success;
    ResourceUploadCommitStatus last_commit_status = ResourceUploadCommitStatus::Success;
    bool has_active_request = false;
};
}
