#pragma once

namespace yuengine::platform {
enum class HOST_STATUS {
    Success,
    StartupFailure,
    TickFailure,
    ShutdownFailure
};
}
