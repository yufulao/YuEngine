// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiPanelRegistryStatus.h

#pragma once

namespace yuengine::uiruntime {
enum class UiPanelRegistryStatus {
    Success,
    InvalidDesc,
    InvalidPanelId,
    DuplicatePanelId,
    PanelNotFound,
    CapacityExceeded,
    InvalidOutputBuffer
};
}
