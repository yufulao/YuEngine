#pragma once

#include "yuengine/file/file_status.h"
#include "yuengine/file/normalized_path.h"

namespace yuengine::file {
struct PathNormalizationResult {
    FileStatus Status;
    NormalizedPath Path;

    static PathNormalizationResult Success(NormalizedPath path);
    static PathNormalizationResult Failure(FileStatus status);
    bool Succeeded() const;
};
}
