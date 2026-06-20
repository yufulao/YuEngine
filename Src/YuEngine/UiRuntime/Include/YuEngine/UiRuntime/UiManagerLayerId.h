// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerLayerId.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerLayerModelConstants.h"

namespace yuengine::uiruntime {
struct UiManagerLayerId final {
    std::uint32_t value = INVALID_UI_MANAGER_LAYER_ID_VALUE;

    /**
     * @comment 检查 UIManager layer id 是否有效。
     * @return 有效时返回 true，否则返回 false。
     */
    bool IsValid() const {
        return value != INVALID_UI_MANAGER_LAYER_ID_VALUE;
    }
};
}
