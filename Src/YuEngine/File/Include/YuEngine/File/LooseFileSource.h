// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/LooseFileSource.h

#pragma once

#include <filesystem>
#include <cstddef>
#include <cstdint>

#include "YuEngine/File/FileReadResult.h"
#include "YuEngine/File/FileWriteResult.h"
#include "YuEngine/File/NormalizedPath.h"

namespace yuengine::file {
class LooseFileSource final {
public:
    /**
     * @comment 构造 LooseFileSource 实例。
     */
    LooseFileSource();
    /**
     * @comment 构造 LooseFileSource 实例。
     * @param root_path 输入 根路径。
     */
    explicit LooseFileSource(std::filesystem::path root_path);

    /**
     * @comment 读取操作。
     * @param path 输入 path。
     * @return 显式操作结果。
     */
    FileReadResult Read(
        NormalizedPath path,
        bool use_range=false,
        std::uint64_t range_byte_offset=0ULL,
        std::uint64_t range_byte_size=0ULL) const;
    /**
     * @comment 写入 字节 到 操作。
     * @param path 输入 path。
     * @param bytes 输入字节。
     * @param byte_count 输入字节数。
     * @return 显式操作结果。
     */
    FileWriteResult Write(NormalizedPath path, const std::uint8_t *bytes, std::size_t byte_count) const;
    /**
     * @comment 返回 根文件系统路径。
     * @return 请求对象的引用。
     */
    const std::filesystem::path& RootPath() const;

private:
    std::filesystem::path root_path_;
};
}
