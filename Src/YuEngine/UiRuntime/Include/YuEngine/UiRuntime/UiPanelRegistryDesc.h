// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiPanelRegistryDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiPanelRegistryConstants.h"

namespace yuengine::uiruntime {
struct UiPanelRegistryDesc final {
    std::uint32_t panel_capacity = MAX_UI_PANEL_REGISTRY_RECORD_COUNT;
};
}
