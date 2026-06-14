#pragma once

#include <cstdint>

#include "YuEngine/Input/InputControlId.h"
#include "YuEngine/Input/InputDeviceId.h"
#include "YuEngine/Input/InputEventType.h"

namespace yuengine::input {
struct InputEvent final {
    InputDeviceId device;
    InputControlId control;
    InputEventType type = InputEventType::ButtonPressed;
    std::int32_t axis_value = 0;
};
}
