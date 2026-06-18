// Module: YuEngine Hardware
// File: Src/YuEngine/Hardware/Include/YuEngine/Hardware/HardwareFrameHostStatus.h

#pragma once

namespace yuengine::hardware {
enum class HardwareFrameHostStatus {
    Success,
    InvalidArgument,
    AlreadyInitialized,
    NotInitialized,
    WindowCreateFailed,
    InputInitializeFailed,
    RhiStorageTooSmall,
    RhiCreateFailed,
    AudioInitializeFailed,
    AudioStartFailed,
    WindowPollFailed,
    InputSubmitFailed,
    InputDrainFailed,
    RenderFrameFailed,
    AudioSubmitFailed,
    AudioWaitFailed,
    AudioDrainFailed,
    ShutdownFailed
};
}
