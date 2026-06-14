#pragma once

#include "yuengine/file/file_status.h"
#include "yuengine/file/normalized_path.h"

namespace yuengine::file {
struct PathNormalizationResult {
    FILE_STATUS Status;
    NormalizedPath Path;

    static PathNormalizationResult Success(NormalizedPath path);
    static PathNormalizationResult Failure(FILE_STATUS status);
    bool Succeeded() const;
};
}
