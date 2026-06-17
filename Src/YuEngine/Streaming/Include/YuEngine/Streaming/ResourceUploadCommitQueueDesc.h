// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadCommitQueueDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Streaming/ResourceUploadCommitConstants.h"

namespace yuengine::streaming {
struct ResourceUploadCommitQueueDesc final {
    std::uint32_t request_capacity = MAX_RESOURCE_UPLOAD_COMMIT_REQUEST_COUNT;
    std::uint32_t completion_capacity = MAX_RESOURCE_UPLOAD_COMMIT_COMPLETION_COUNT;
};
}
