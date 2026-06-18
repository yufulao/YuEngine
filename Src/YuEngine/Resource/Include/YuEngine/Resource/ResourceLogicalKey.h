// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceLogicalKey.h

#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include "YuEngine/Resource/ResourceConstants.h"

namespace yuengine::resource {
class ResourceLogicalKey final {
public:
    /**
     * @comment 构造 ResourceLogicalKey 实例。
     */
    ResourceLogicalKey();
    /**
     * @comment 构造 ResourceLogicalKey 实例。
     * @param value 输入 值。
     */
    explicit ResourceLogicalKey(std::string_view value);

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
    /**
     * @comment 检查值是否在范围内。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsWithinBounds() const;
    /**
     * @comment 检查值是否相等。
     * @param other 输入比较对象。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Equals(const ResourceLogicalKey& other) const;

private:
    std::array<char, MAX_LOGICAL_KEY_BYTES> bytes_;
    std::size_t length_;
    bool is_valid_;
    bool is_within_bounds_;
};
}
