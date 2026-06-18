// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/FileReadResult.h

#pragma once

#include <cstdint>
#include <vector>

#include "YuEngine/File/FileStatus.h"

namespace yuengine::file {
struct FileReadResult {
    FileStatus status;
    std::vector<std::uint8_t> bytes;

    /**
     * @comment 创建成功结果。
     * @param bytes 输入字节数或字节载荷。
     * @return 显式操作结果。
     */
    static FileReadResult Success(std::vector<std::uint8_t> bytes);
    /**
     * @comment 创建失败结果。
     * @param status 输入 状态。
     * @return 显式操作结果。
     */
    static FileReadResult Failure(FileStatus status);
    /**
     * @comment 检查结果是否成功。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Succeeded() const;
};
}
