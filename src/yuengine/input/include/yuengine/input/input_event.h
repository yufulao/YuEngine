#pragma once

#include <cstdint>

#include "yuengine/input/input_control_id.h"
#include "yuengine/input/input_device_id.h"
#include "yuengine/input/input_event_type.h"

namespace yuengine::input {
struct InputEvent final {
    InputDeviceId Device;
    InputControlId Control;
    InputEventType Type = InputEventType::ButtonPressed;
    std::int32_t AxisValue = 0;
};
}
