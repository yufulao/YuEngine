// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadQueueDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Streaming/ResourceUploadConstants.h"

namespace yuengine::streaming {
struct ResourceUploadQueueDesc final {
    std::uint32_t request_capacity = MAX_RESOURCE_UPLOAD_REQUEST_COUNT;
    std::uint32_t completion_capacity = MAX_RESOURCE_UPLOAD_COMPLETION_COUNT;
};
}
