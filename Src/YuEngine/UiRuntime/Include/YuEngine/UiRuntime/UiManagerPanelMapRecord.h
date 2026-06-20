// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerPanelMapRecord.h

#pragma once

#include "YuEngine/UiRuntime/BaseUiController.h"
#include "YuEngine/UiRuntime/UiManagerLayerRecord.h"
#include "YuEngine/UiRuntime/UiPanelManifestRecord.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgsSnapshot.h"

namespace yuengine::uiruntime {
struct UiManagerPanelMapRecord final {
    UiPanelId panel_id;
    UiPanelManifestRecord manifest_record;
    UiManagerLayerRecord layer_record;
    UiPanelOpenArgsSnapshot open_args;
    BaseUiController *controller = nullptr;
    bool loaded = false;
    bool active = false;

    bool IsValid() const {
        if (!panel_id.IsValid()) {
            return false;
        }

        if (!manifest_record.IsValid()) {
            return false;
        }

        if (!layer_record.IsValid()) {
            return false;
        }

        return controller != nullptr;
    }
};
}
