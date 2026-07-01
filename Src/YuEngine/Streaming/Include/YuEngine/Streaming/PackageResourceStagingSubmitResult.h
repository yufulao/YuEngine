// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/PackageResourceStagingSubmitResult.h

#pragma once

#include <cstdint>

#include "YuEngine/File/AsyncFileReadStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Streaming/PackageResourceStagingStatus.h"

namespace yuengine::streaming {
struct PackageResourceStagingSubmitResult final {
    PackageResourceStagingStatus status = PackageResourceStagingStatus::Success;
    file::AsyncFileReadStatus async_file_status = file::AsyncFileReadStatus::Success;
    std::uint64_t request_id = 0U;
};

struct PackageResourceStagingBatchSubmitResult final {
    PackageResourceStagingStatus status = PackageResourceStagingStatus::Success;
    resource::ResourceStatus resource_status = resource::ResourceStatus::Success;
    file::AsyncFileReadStatus async_file_status = file::AsyncFileReadStatus::Success;
    std::uint32_t request_count = 0U;
    std::uint32_t submitted_count = 0U;
    std::uint32_t required_result_count = 0U;
    std::uint32_t failed_index = 0U;
    std::uint64_t failed_request_id = 0U;
};
}
