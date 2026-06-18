// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/AsyncFileReadResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/File/AsyncFileReadStatus.h"
#include "YuEngine/File/FileStatus.h"

namespace yuengine::file {
struct AsyncFileReadResult {
    AsyncFileReadStatus status = AsyncFileReadStatus::NotInitialized;
    FileStatus file_status = FileStatus::Success;
    std::uint64_t request_index = 0U;
    std::size_t byte_count = 0U;
};
}
