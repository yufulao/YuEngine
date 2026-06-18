// 模块: YuEngine Object
// 文件: Src/YuEngine/Object/Include/YuEngine/Object/ObjectTypeId.h

#pragma once

#include <cstdint>

namespace yuengine::object {
struct ObjectTypeId final {
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
