// 模块: YuEngine Object
// 文件: Src/YuEngine/Object/Include/YuEngine/Object/ObjectHandle.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectConstants.h"

namespace yuengine::object {
struct ObjectHandle final {
    std::uint32_t slot = INVALID_OBJECT_SLOT;
    std::uint32_t generation = INVALID_OBJECT_GENERATION;

    /**
     * @comment 检查 value 是否有效。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const {
        if (slot == INVALID_OBJECT_SLOT) {
            return false;
        }

        return generation != INVALID_OBJECT_GENERATION;
    }
};
}
