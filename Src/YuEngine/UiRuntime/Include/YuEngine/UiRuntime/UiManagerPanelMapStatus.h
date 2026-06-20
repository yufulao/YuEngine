// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerPanelMapStatus.h

#pragma once

namespace yuengine::uiruntime {
enum class UiManagerPanelMapStatus {
    Success,
    InvalidPanelId,
    InvalidController,
    InvalidOutputBuffer,
    PanelNotRegistered,
    PanelNotLoaded,
    PanelNotActive,
    PanelLayerNotBound,
    LayerNotFound,
    InvalidOpenArgs,
    ControllerOpenFailed,
    ControllerCloseFailed,
    ControllerReleaseFailed,
    CapacityExceeded
};
}
