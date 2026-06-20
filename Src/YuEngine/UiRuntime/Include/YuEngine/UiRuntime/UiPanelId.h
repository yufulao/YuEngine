// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiPanelId.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiPanelRegistryConstants.h"

namespace yuengine::uiruntime {
struct UiPanelId final {
    std::uint32_t value = INVALID_UI_PANEL_ID_VALUE;

    /**
     * @comment 检查 Panel ID 是否有效。
     * @return 有效时返回 true，否则返回 false。
     */
    bool IsValid() const {
        return value != INVALID_UI_PANEL_ID_VALUE;
    }
};
}
