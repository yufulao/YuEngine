// 模块: YuEngine File
// 文件: Src/YuEngine/File/Src/PathNormalizationResult.cpp

#include "YuEngine/File/PathNormalizationResult.h"

#include <utility>

namespace yuengine::file {
PathNormalizationResult PathNormalizationResult::Success(NormalizedPath path) {
    return PathNormalizationResult{FileStatus::Success, std::move(path)};
}

PathNormalizationResult PathNormalizationResult::Failure(FileStatus status) {
    return PathNormalizationResult{status, NormalizedPath()};
}

bool PathNormalizationResult::Succeeded() const {
    return status == FileStatus::Success;
}
}
