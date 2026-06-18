// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Include/YuEngine/Package/PackageId.h

#pragma once

#include <cstdint>

namespace yuengine::package {
struct PackageId final {
    std::uint32_t value = 0U;

    /**
     * @comment 检查 value 是否有效。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const {
        return value != 0U;
    }
};
}
