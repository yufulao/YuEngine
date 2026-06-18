// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceHandle.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"

namespace yuengine::resource {
struct ResourceHandle final {
    std::uint32_t slot = INVALID_RESOURCE_SLOT;
    std::uint32_t generation = INVALID_RESOURCE_GENERATION;

    /**
     * @comment 检查值是否合法。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const {
        if (slot == INVALID_RESOURCE_SLOT) {
            return false;
        }

        return generation != INVALID_RESOURCE_GENERATION;
    }
};
}
