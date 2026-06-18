// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Include/YuEngine/Package/PackageSourceKey.h

#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include "YuEngine/Package/PackageConstants.h"

namespace yuengine::package {
class PackageSourceKey final {
public:
    /**
     * @comment 构造 PackageSourceKey 实例。
     */
    PackageSourceKey();
    /**
     * @comment 构造 PackageSourceKey 实例。
     * @param value 输入 value。
     */
    explicit PackageSourceKey(std::string_view value);

    /**
     * @comment 返回 stored value。
     * @return Value 值。
     */
    std::string_view Value() const;
    /**
     * @comment 返回 stored byte length。
     * @return Byte length 值。
     */
    std::size_t ByteLength() const;
    /**
     * @comment 检查 value 是否有效。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const;
    /**
     * @comment 检查 value 是否在范围内。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsWithinBounds() const;
    /**
     * @comment 检查 values 是否相等。
     * @param other 输入 other。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Equals(const PackageSourceKey& other) const;

private:
    std::array<char, MAX_PACKAGE_SOURCE_KEY_BYTES> bytes_;
    std::size_t length_;
    bool is_valid_;
    bool is_within_bounds_;
};
}
