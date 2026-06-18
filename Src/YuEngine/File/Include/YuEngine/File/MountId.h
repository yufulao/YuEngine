// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/MountId.h

#pragma once

#include <string>
#include <string_view>

namespace yuengine::file {
class MountId final {
public:
    /**
     * @comment 构造 MountId 实例。
     */
    MountId();
    /**
     * @comment 构造 MountId 实例。
     * @param value 输入 值。
     */
    explicit MountId(std::string value);

    /**
     * @comment 返回保存的值。
     * @return 值 值。
     */
    std::string_view Value() const;
    /**
     * @comment 检查值是否合法。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const;

private:
    std::string value_;
};
}
