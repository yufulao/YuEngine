#pragma once

#include <cstdint>

#include "yuengine/input/input_control_id.h"
#include "yuengine/input/input_device_id.h"
#include "yuengine/input/input_event_type.h"

namespace yuengine::input {
struct input_event_t final {
    input_device_id_t Device;
    input_control_id_t Control;
    InputEventType Type = InputEventType::ButtonPressed;
    std::int32_t AxisValue = 0;
};
}
