// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowEventType.h

#pragma once

namespace yuengine::platform {
enum class PlatformWindowEventType {
    None,
    CloseRequested,
    Resized,
    FocusGained,
    FocusLost,
    Minimized,
    Restored,
    RawKeyDown,
    RawKeyUp,
    RawMouseMove,
    RawMouseButtonDown,
    RawMouseButtonUp
};
}
