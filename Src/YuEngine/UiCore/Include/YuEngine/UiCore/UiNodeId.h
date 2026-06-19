// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiNodeId.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiCoreConstants.h"

namespace yuengine::uicore {
struct UiNodeId final {
    std::uint32_t value = INVALID_UI_NODE_ID_VALUE;

    /**
     * @comment 检查 UI node id 是否有效。
     * @return id 有效时返回 true，否则返回 false。
     */
    bool IsValid() const {
        return value != INVALID_UI_NODE_ID_VALUE;
    }
};
}
