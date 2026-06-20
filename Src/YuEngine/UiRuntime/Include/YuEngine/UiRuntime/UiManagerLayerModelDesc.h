// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerLayerModelDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerLayerModelConstants.h"

namespace yuengine::uiruntime {
struct UiManagerLayerModelDesc final {
    std::uint32_t layer_capacity = MAX_UI_MANAGER_LAYER_COUNT;
    std::uint32_t binding_capacity = MAX_UI_MANAGER_PANEL_LAYER_BINDING_COUNT;
};
}
