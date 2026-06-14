#include "yuengine/file/file_read_result.h"

#include <utility>

namespace yuengine::file {
file_read_result_t file_read_result_t::Success(std::vector<std::uint8_t> bytes) {
    return file_read_result_t{FileStatus::Success, std::move(bytes)};
}

file_read_result_t file_read_result_t::Failure(FileStatus status) {
    return file_read_result_t{status, {}};
}

bool file_read_result_t::Succeeded() const {
    return Status == FileStatus::Success;
}
}
