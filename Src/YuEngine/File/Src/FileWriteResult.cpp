// Module: YuEngine File
// File: Src/YuEngine/File/Src/FileWriteResult.cpp

#include "YuEngine/File/FileWriteResult.h"

namespace yuengine::file {
FileWriteResult FileWriteResult::Success(std::size_t byte_count) {
    return FileWriteResult{FileStatus::Success, byte_count};
}

FileWriteResult FileWriteResult::Failure(FileStatus status) {
    return FileWriteResult{status, 0U};
}

bool FileWriteResult::Succeeded() const {
    return status == FileStatus::Success;
}
}
