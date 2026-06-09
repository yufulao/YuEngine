#pragma once

namespace yuengine::input
{
enum class InputStatus
{
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
