// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/PackageResourceStagingSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/File/AsyncFileReadStatus.h"
#include "YuEngine/File/FileStatus.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Streaming/PackageResourceStagingStatus.h"

namespace yuengine::streaming {
struct PackageResourceStagingSnapshot final {
    std::uint32_t request_capacity = 0U;
    std::uint32_t completion_capacity = 0U;
    std::uint32_t pending_count = 0U;
    std::uint32_t completion_count = 0U;
    std::uint32_t max_pending_count = 0U;
    std::uint32_t max_completion_count = 0U;
    std::uint64_t submitted_count = 0U;
    std::uint64_t completed_count = 0U;
    std::uint64_t failed_count = 0U;
    std::uint64_t rejected_count = 0U;
    std::uint64_t duplicate_request_count = 0U;
    std::uint64_t file_submit_failed_count = 0U;
    std::uint64_t missing_completion_count = 0U;
    PackageResourceStagingStatus last_status = PackageResourceStagingStatus::Success;
    resource::ResourceStatus last_resource_status = resource::ResourceStatus::Success;
    file::AsyncFileReadStatus last_async_file_status = file::AsyncFileReadStatus::Success;
    file::FileStatus last_file_status = file::FileStatus::Success;
    memory::MemoryAccountingStatus allocation_accounting_status =
        memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
};
}
