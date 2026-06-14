#pragma once

#include <cstdint>

#include "YuEngine/Input/InputControlId.h"
#include "YuEngine/Input/InputDeviceId.h"
#include "YuEngine/Input/InputEventType.h"

namespace yuengine::input {
struct InputEvent final {
    InputDeviceId Device;
    InputControlId Control;
    InputEventType Type = InputEventType::ButtonPressed;
    std::int32_t AxisValue = 0;
};
}
