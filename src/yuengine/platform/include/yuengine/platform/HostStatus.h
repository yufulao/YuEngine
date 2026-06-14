#pragma once

namespace yuengine::platform {
enum class HostStatus {
    Success,
    StartupFailure,
    TickFailure,
    ShutdownFailure
};
}
