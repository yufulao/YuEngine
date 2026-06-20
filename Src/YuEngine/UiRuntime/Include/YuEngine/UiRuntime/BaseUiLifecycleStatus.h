// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/BaseUiLifecycleStatus.h

#pragma once

namespace yuengine::uiruntime {
enum class BaseUiLifecycleStatus {
    Success,
    InvalidController,
    InvalidCloseRequestSink,
    AlreadyOpen,
    AlreadyDestroyed,
    NotOpen,
    Destroyed,
    HookFailed
};
}
