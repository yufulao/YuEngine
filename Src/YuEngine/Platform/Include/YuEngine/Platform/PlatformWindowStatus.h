// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowStatus.h

#pragma once

namespace yuengine::platform {
enum class PlatformWindowStatus {
    Success,
    InvalidDescriptor,
    NullPointer,
    Unsupported,
    AlreadyCreated,
    NotCreated,
    Closed,
    InvalidLifecycle,
    NativeCallFailed,
    EventQueueOverflow,
    OutputBufferFull
};
}
