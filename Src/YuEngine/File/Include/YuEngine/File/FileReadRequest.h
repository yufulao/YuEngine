// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/FileReadRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/File/MountId.h"
#include "YuEngine/File/VirtualPath.h"

namespace yuengine::file {
struct FileReadRequest {
    MountId mount;
    VirtualPath path;
    bool use_range = false;
    std::uint64_t range_byte_offset = 0ULL;
    std::uint64_t range_byte_size = 0ULL;
};
}
