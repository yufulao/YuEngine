// 模块: YuEngine File
// 文件: Src/YuEngine/File/Src/FileReadResult.cpp

#include "YuEngine/File/FileReadResult.h"

#include <utility>

namespace yuengine::file {
FileReadResult FileReadResult::Success(std::vector<std::uint8_t> bytes) {
    return FileReadResult{FileStatus::Success, std::move(bytes)};
}

FileReadResult FileReadResult::Failure(FileStatus status) {
    return FileReadResult{status, {}};
}

bool FileReadResult::Succeeded() const {
    return status == FileStatus::Success;
}
}
