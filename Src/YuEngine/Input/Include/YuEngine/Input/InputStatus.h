// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputStatus.h

#pragma once

namespace yuengine::input {
enum class InputStatus {
    Success,
    DuplicateBinding,
    CapacityExceeded,
    UnknownDeviceControl,
    UnknownAction,
    InvalidAxisValue,
    InvalidEvent,
    EndOfReplay,
    NotInitialized,
    AlreadyInitialized,
    InvalidDescriptor,
    InvalidUserIndex = InvalidDescriptor,
    UnsupportedBackend,
    SourceUnavailable,
    DeviceUnavailable = SourceUnavailable,
    FocusLost,
    NullPointer,
    OutputBufferFull,
    BackendError
};
}
