// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerPopupStackStatus.h

#pragma once

namespace yuengine::uiruntime {
enum class UiManagerPopupStackStatus {
    Success,
    InvalidPanelId,
    InvalidPanelMap,
    InvalidController,
    InvalidOutputBuffer,
    PanelNotRegistered,
    PanelNotLoaded,
    PanelNotActive,
    PanelLayerNotBound,
    LayerNotFound,
    NonPopupPanel,
    PopupNotInStack,
    ControllerOpenFailed,
    ControllerCloseFailed,
    CapacityExceeded,
    PanelMapRejected
};
}
