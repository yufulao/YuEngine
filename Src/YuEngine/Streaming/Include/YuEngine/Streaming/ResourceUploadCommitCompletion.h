// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadCommitCompletion.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Streaming/ResourceUploadCommitStatus.h"
#include "YuEngine/Streaming/ResourceUploadStatus.h"

namespace yuengine::streaming {
struct ResourceUploadCommitCompletion final {
    ResourceUploadCommitStatus status = ResourceUploadCommitStatus::Success;
    resource::ResourceLoadCommitStatus resource_commit_status =
        resource::ResourceLoadCommitStatus::Success;
    resource::ResourceLoadState load_state = resource::ResourceLoadState::Unloaded;
    ResourceUploadStatus upload_status = ResourceUploadStatus::Success;
    resource::ResourceHandle resource;
    resource::ResourceTypeId expected_type;
    std::uint64_t commit_id = 0U;
    std::uint64_t upload_id = 0U;
    std::uint64_t staging_request_id = 0U;
    std::uint32_t upload_byte_count = 0U;
    rhi::RhiStatus rhi_status = rhi::RhiStatus::Success;
};
}
