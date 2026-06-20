// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerFullscreenStackStatus.h

#pragma once

namespace yuengine::uiruntime {
enum class UiManagerFullscreenStackStatus {
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
    NonFullscreenPanel,
    FullscreenNotInStack,
    BackStackEmpty,
    ControllerOpenFailed,
    ControllerCloseFailed,
    CapacityExceeded,
    PanelMapRejected
};
}
