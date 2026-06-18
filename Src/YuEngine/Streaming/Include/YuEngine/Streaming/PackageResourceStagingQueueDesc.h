// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/PackageResourceStagingQueueDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Streaming/PackageResourceStagingConstants.h"

namespace yuengine::streaming {
struct PackageResourceStagingQueueDesc final {
    std::uint32_t request_capacity = MAX_PACKAGE_RESOURCE_STAGING_REQUEST_COUNT;
    std::uint32_t completion_capacity = MAX_PACKAGE_RESOURCE_STAGING_COMPLETION_COUNT;
};
}
