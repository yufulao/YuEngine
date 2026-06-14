#pragma once

namespace yuengine::input {
enum class INPUT_STATUS {
    Success,
    DuplicateBinding,
    CapacityExceeded,
    UnknownDeviceControl,
    UnknownAction,
    InvalidAxisValue,
    InvalidEvent,
    EndOfReplay
};
}
