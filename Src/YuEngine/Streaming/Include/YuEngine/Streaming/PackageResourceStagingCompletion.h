// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/PackageResourceStagingCompletion.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/File/AsyncFileReadStatus.h"
#include "YuEngine/File/FileStatus.h"
#include "YuEngine/Package/PackageLoadPlanRecord.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Streaming/PackageResourceStagingStatus.h"

namespace yuengine::streaming {
struct PackageResourceStagingCompletion final {
    PackageResourceStagingStatus status = PackageResourceStagingStatus::Success;
    package::PackageLoadPlanRecord package_record;
    resource::ResourceHandle resource;
    resource::ResourceTypeId expected_type;
    std::uint64_t request_id = 0U;
    std::size_t file_byte_count = 0U;
    std::uint32_t staged_byte_offset = 0U;
    std::uint32_t staged_byte_count = 0U;
    resource::ResourceStatus resource_status = resource::ResourceStatus::Success;
    file::AsyncFileReadStatus async_file_status = file::AsyncFileReadStatus::Success;
    file::FileStatus file_status = file::FileStatus::Success;
};
}
