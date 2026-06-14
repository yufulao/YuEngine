#include "yuengine/file/file_read_result.h"

#include <utility>

namespace yuengine::file {
FileReadResult FileReadResult::Success(std::vector<std::uint8_t> bytes) {
    return FileReadResult{FILE_STATUS::Success, std::move(bytes)};
}

FileReadResult FileReadResult::Failure(FILE_STATUS status) {
    return FileReadResult{status, {}};
}

bool FileReadResult::Succeeded() const {
    return Status == FILE_STATUS::Success;
}
}
