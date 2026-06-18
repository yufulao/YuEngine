// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldObjectId.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"

namespace yuengine::world {
struct WorldObjectId final {
    std::uint32_t value = INVALID_WORLD_OBJECT_ID_VALUE;

    /**
     * @comment 检查 world object id 是否有效。
     * @return id 有效时返回 true，否则返回 false。
     */
    bool IsValid() const {
        return value != INVALID_WORLD_OBJECT_ID_VALUE;
    }
};
}
