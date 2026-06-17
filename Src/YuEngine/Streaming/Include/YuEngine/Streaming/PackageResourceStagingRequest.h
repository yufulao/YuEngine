// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/PackageResourceStagingRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/File/AsyncFileReadRequest.h"
#include "YuEngine/Package/PackageLoadPlanRecord.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::file {
class AsyncFileReadQueue;
}

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::streaming {
struct PackageResourceStagingRequest final {
    const resource::ResourceRegistry *resource_registry = nullptr;
    file::AsyncFileReadQueue *file_queue = nullptr;
    package::PackageLoadPlanRecord package_record;
    resource::ResourceHandle resource;
    resource::ResourceTypeId expected_type;
    file::AsyncFileReadRequest file_request;
    std::uint64_t request_id = 0U;
};
}
