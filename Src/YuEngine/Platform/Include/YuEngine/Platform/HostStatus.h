// Module: YuEngine Platform
// File: Src/YuEngine/Platform/Include/YuEngine/Platform/HostStatus.h

#pragma once

namespace yuengine::platform {
enum class HostStatus {
    Success,
    StartupFailure,
    TickFailure,
    ShutdownFailure
};
}
