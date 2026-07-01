// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/PackageResourceStagingPendingRequestSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Package/PackageLoadPlanRecord.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Streaming/PackageResourceStagingStatus.h"

namespace yuengine::streaming {
struct PackageResourceStagingPendingRequestSnapshot final {
    package::PackageLoadPlanRecord package_record;
    resource::ResourceHandle resource;
    resource::ResourceTypeId expected_type;
    std::uint64_t request_id = 0U;
};

struct PackageResourceStagingPendingRequestEnumerationResult final {
    PackageResourceStagingStatus status = PackageResourceStagingStatus::Success;
    std::uint32_t required_request_count = 0U;
    std::uint32_t written_count = 0U;
};
}
