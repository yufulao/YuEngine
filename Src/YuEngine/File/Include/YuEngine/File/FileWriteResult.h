// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/FileWriteResult.h

#pragma once

#include <cstddef>

#include "YuEngine/File/FileStatus.h"

namespace yuengine::file {
struct FileWriteResult {
    FileStatus status = FileStatus::WriteFailure;
    std::size_t byte_count = 0U;

    static FileWriteResult Success(std::size_t byte_count);
    static FileWriteResult Failure(FileStatus status);
    bool Succeeded() const;
};
}
