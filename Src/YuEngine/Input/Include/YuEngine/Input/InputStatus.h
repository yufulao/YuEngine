// Module: YuEngine Input
// File: Src/YuEngine/Input/Include/YuEngine/Input/InputStatus.h

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
    UnsupportedBackend,
    SourceUnavailable,
    FocusLost,
    NullPointer,
    OutputBufferFull
};
}
