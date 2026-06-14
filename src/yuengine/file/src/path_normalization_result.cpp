#include "yuengine/file/path_normalization_result.h"

#include <utility>

namespace yuengine::file {
path_normalization_result_t path_normalization_result_t::Success(NormalizedPath path) {
    return path_normalization_result_t{FileStatus::Success, std::move(path)};
}

path_normalization_result_t path_normalization_result_t::Failure(FileStatus status) {
    return path_normalization_result_t{status, NormalizedPath()};
}

bool path_normalization_result_t::Succeeded() const {
    return Status == FileStatus::Success;
}
}
