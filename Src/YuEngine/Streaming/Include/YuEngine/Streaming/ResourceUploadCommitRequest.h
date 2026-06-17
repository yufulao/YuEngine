// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadCommitRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/Streaming/ResourceUploadCompletion.h"

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::streaming {
struct ResourceUploadCommitRequest final {
    resource::ResourceRegistry *resource_registry = nullptr;
    ResourceUploadCompletion upload_completion;
    std::uint64_t commit_id = 0U;
};
}
