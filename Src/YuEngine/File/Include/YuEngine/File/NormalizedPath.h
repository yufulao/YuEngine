// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/NormalizedPath.h

#pragma once

#include <string>
#include <string_view>

namespace yuengine::file {
class NormalizedPath final {
public:
    /**
     * @comment 构造 NormalizedPath 实例。
     */
    NormalizedPath();
    /**
     * @comment 构造 NormalizedPath 实例。
     * @param value 输入 值。
     */
    explicit NormalizedPath(std::string value);

    /**
     * @comment 返回保存的值。
     * @return 值 值。
     */
    std::string_view Value() const;
    /**
     * @comment 返回保存的字节长度。
     * @return 字节 length 值。
     */
    std::size_t ByteLength() const;
    /**
     * @comment 检查值是否合法。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const;

private:
    std::string value_;
};
}
