#include "yuengine/file/file_read_result.h"

#include <utility>

namespace yuengine::file {
file_read_result_t file_read_result_t::Success(std::vector<std::uint8_t> bytes) {
    return file_read_result_t{FILE_STATUS::Success, std::move(bytes)};
}

file_read_result_t file_read_result_t::Failure(FILE_STATUS status) {
    return file_read_result_t{status, {}};
}

bool file_read_result_t::Succeeded() const {
    return Status == FILE_STATUS::Success;
}
}
