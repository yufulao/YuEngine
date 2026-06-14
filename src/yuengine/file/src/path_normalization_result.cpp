#include "yuengine/file/path_normalization_result.h"

#include <utility>

namespace yuengine::file {
PathNormalizationResult PathNormalizationResult::Success(NormalizedPath path) {
    return PathNormalizationResult{FILE_STATUS::Success, std::move(path)};
}

PathNormalizationResult PathNormalizationResult::Failure(FILE_STATUS status) {
    return PathNormalizationResult{status, NormalizedPath()};
}

bool PathNormalizationResult::Succeeded() const {
    return Status == FILE_STATUS::Success;
}
}
