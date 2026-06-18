// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/FileWriteRequest.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/File/MountId.h"
#include "YuEngine/File/VirtualPath.h"

namespace yuengine::file {
struct FileWriteRequest {
    MountId mount;
    VirtualPath path;
    const std::uint8_t *bytes = nullptr;
    std::size_t byte_count = 0U;
};
}
