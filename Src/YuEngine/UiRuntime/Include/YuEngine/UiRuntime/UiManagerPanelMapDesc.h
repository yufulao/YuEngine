// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerPanelMapDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerPanelMapConstants.h"

namespace yuengine::uiruntime {
struct UiManagerPanelMapDesc final {
    std::uint32_t panel_capacity = MAX_UI_MANAGER_PANEL_MAP_RECORD_COUNT;
};
}
