#pragma once

#include "YuEngine/File/FileStatus.h"
#include "YuEngine/File/NormalizedPath.h"

namespace yuengine::file {
struct PathNormalizationResult {
    FileStatus status;
    NormalizedPath path;

    static PathNormalizationResult Success(NormalizedPath path);
    static PathNormalizationResult Failure(FileStatus status);
    bool Succeeded() const;
};
}
