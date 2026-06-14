#pragma once

#include "yuengine/file/file_status.h"
#include "yuengine/file/normalized_path.h"

namespace yuengine::file {
struct path_normalization_result_t {
    FILE_STATUS Status;
    NormalizedPath Path;

    static path_normalization_result_t Success(NormalizedPath path);
    static path_normalization_result_t Failure(FILE_STATUS status);
    bool Succeeded() const;
};
}
