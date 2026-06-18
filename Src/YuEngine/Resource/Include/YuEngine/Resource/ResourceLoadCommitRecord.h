// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceLoadCommitRecord.h

#pragma once

#include "YuEngine/Resource/ResourceLoadCommitRequest.h"

namespace yuengine::resource {
struct ResourceLoadCommitRecord final {
    ResourceLoadCommitRequest request;
    bool is_active = false;
};
}
