#include "yuengine/file/file_read_result.h"

#include <utility>

namespace yuengine::file {
FileReadResult FileReadResult::Success(std::vector<std::uint8_t> bytes) {
    return FileReadResult{FileStatus::Success, std::move(bytes)};
}

FileReadResult FileReadResult::Failure(FileStatus status) {
    return FileReadResult{status, {}};
}

bool FileReadResult::Succeeded() const {
    return Status == FileStatus::Success;
}
}
