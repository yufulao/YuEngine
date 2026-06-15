// Module: YuEngine File
// File: Src/YuEngine/File/Include/YuEngine/File/PathNormalizationResult.h

#pragma once

#include "YuEngine/File/FileStatus.h"
#include "YuEngine/File/NormalizedPath.h"

namespace yuengine::file {
struct PathNormalizationResult {
    FileStatus status;
    NormalizedPath path;

    /**
     * @comment Creates a successful result.
     * @param path Input path.
     * @return Explicit operation result.
     */
    static PathNormalizationResult Success(NormalizedPath path);
    /**
     * @comment Creates a failed result.
     * @param status Input status.
     * @return Explicit operation result.
     */
    static PathNormalizationResult Failure(FileStatus status);
    /**
     * @comment Checks whether the result succeeded.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool Succeeded() const;
};
}
