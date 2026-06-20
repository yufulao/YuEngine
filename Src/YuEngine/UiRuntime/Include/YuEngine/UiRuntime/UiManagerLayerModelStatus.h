// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerLayerModelStatus.h

#pragma once

namespace yuengine::uiruntime {
enum class UiManagerLayerModelStatus {
    Success,
    InvalidDesc,
    InvalidLayerId,
    InvalidLayerType,
    InvalidLayerRoot,
    DuplicateLayerId,
    DuplicateLayerType,
    LayerNotFound,
    InvalidPanelId,
    DuplicatePanelId,
    PanelNotBound,
    CapacityExceeded,
    InvalidOutputBuffer
};
}
