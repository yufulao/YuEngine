// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/PathNormalizationResult.h

#pragma once

#include "YuEngine/File/FileStatus.h"
#include "YuEngine/File/NormalizedPath.h"

namespace yuengine::file {
struct PathNormalizationResult {
    FileStatus status;
    NormalizedPath path;

    /**
     * @comment 创建成功结果。
     * @param path 输入 path。
     * @return 显式操作结果。
     */
    static PathNormalizationResult Success(NormalizedPath path);
    /**
     * @comment 创建失败结果。
     * @param status 输入 状态。
     * @return 显式操作结果。
     */
    static PathNormalizationResult Failure(FileStatus status);
    /**
     * @comment 检查结果是否成功。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Succeeded() const;
};
}
