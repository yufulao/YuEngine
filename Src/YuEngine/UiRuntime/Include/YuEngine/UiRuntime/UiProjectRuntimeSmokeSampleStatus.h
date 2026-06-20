// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiProjectRuntimeSmokeSampleStatus.h

#pragma once

namespace yuengine::uiruntime {
enum class UiProjectRuntimeSmokeSampleStatus {
    Success,
    InvalidOutputBuffer,
    SetupFailed,
    PopupOpenFailed,
    PopupDisplayMismatch,
    PopupCloseFailed,
    PopupReleaseFailed,
    FullscreenFirstOpenFailed,
    FullscreenSecondOpenFailed,
    FullscreenBackFailed,
    FullscreenDisplayMismatch,
    FullscreenReleaseFailed,
    GridOpenFailed,
    GridDisplayMismatch,
    GridReleaseFailed,
    CleanupMismatch
};
}
