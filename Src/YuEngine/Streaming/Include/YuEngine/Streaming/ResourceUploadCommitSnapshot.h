// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadCommitSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Streaming/ResourceUploadCommitStatus.h"

namespace yuengine::streaming {
struct ResourceUploadCommitSnapshot final {
    std::uint32_t request_capacity = 0U;
    std::uint32_t completion_capacity = 0U;
    std::uint32_t pending_count = 0U;
    std::uint32_t completion_count = 0U;
    std::uint32_t max_pending_count = 0U;
    std::uint32_t max_completion_count = 0U;
    std::uint64_t submitted_count = 0U;
    std::uint64_t committed_count = 0U;
    std::uint64_t failed_upload_commit_count = 0U;
    std::uint64_t rejected_count = 0U;
    std::uint64_t duplicate_commit_count = 0U;
    std::uint64_t resource_commit_failed_count = 0U;
    ResourceUploadCommitStatus last_status = ResourceUploadCommitStatus::Success;
    resource::ResourceLoadCommitStatus last_resource_commit_status =
        resource::ResourceLoadCommitStatus::Success;
    resource::ResourceLoadState last_load_state = resource::ResourceLoadState::Unloaded;
    memory::MemoryAccountingStatus allocation_accounting_status =
        memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    std::uint64_t last_failed_upload_commit_id = 0U;
    std::uint64_t last_failed_upload_commit_upload_id = 0U;
    resource::ResourceHandle last_failed_upload_commit_resource;
    resource::ResourceTypeId last_failed_upload_commit_expected_type;
    std::uint32_t last_failed_upload_commit_request_capacity = 0U;
    std::uint32_t last_failed_upload_commit_completion_capacity = 0U;
    std::uint32_t last_failed_upload_commit_pending_count = 0U;
    std::uint32_t last_failed_upload_commit_completion_count = 0U;
    std::uint32_t last_required_upload_commit_request_count = 0U;
    std::uint32_t last_required_upload_commit_completion_count = 0U;
};
}
