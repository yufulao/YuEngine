#pragma once

#include "yuengine/file/FileStatus.h"
#include "yuengine/file/NormalizedPath.h"

namespace yuengine::file {
struct PathNormalizationResult {
    FileStatus Status;
    NormalizedPath Path;

    static PathNormalizationResult Success(NormalizedPath path);
    static PathNormalizationResult Failure(FileStatus status);
    bool Succeeded() const;
};
}
