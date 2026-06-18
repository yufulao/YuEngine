// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceLoadCommitRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceLoadCommitRequest final {
    ResourceHandle resource;
    ResourceTypeId expected_type;
    ResourceLoadState load_state = ResourceLoadState::Unloaded;
    std::uint64_t commit_id = 0U;
    std::uint64_t upload_id = 0U;
    std::uint64_t staging_request_id = 0U;
    std::uint32_t upload_byte_count = 0U;
};
}
