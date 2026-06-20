// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerPanelLayerBinding.h

#pragma once

#include "YuEngine/UiRuntime/UiManagerLayerId.h"
#include "YuEngine/UiRuntime/UiPanelId.h"

namespace yuengine::uiruntime {
struct UiManagerPanelLayerBinding final {
    UiPanelId panel_id;
    UiManagerLayerId layer_id;

    bool IsValid() const {
        if (!panel_id.IsValid()) {
            return false;
        }

        return layer_id.IsValid();
    }
};
}
