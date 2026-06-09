#pragma once

#include <cstdint>

#include "yuengine/input/InputControlId.h"
#include "yuengine/input/InputDeviceId.h"
#include "yuengine/input/InputEventType.h"

namespace yuengine::input
{
struct InputEvent final
{
    InputDeviceId Device;
    InputControlId Control;
    InputEventType Type = InputEventType::ButtonPressed;
    std::int32_t AxisValue = 0;
};
}
