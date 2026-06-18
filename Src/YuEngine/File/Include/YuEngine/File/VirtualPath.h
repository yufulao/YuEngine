// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/VirtualPath.h

#pragma once

#include <string>
#include <string_view>

namespace yuengine::file {
class VirtualPath final {
public:
    /**
     * @comment 构造 VirtualPath 实例。
     */
    VirtualPath();
    /**
     * @comment 构造 VirtualPath 实例。
     * @param value 输入 值。
     */
    explicit VirtualPath(std::string value);

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

private:
    std::string value_;
};
}
