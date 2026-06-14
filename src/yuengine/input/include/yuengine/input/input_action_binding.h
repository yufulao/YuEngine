#pragma once

#include "yuengine/input/input_action_id.h"
#include "yuengine/input/input_control_id.h"
#include "yuengine/input/input_device_id.h"

namespace yuengine::input {
struct InputActionBinding final {
    InputDeviceId Device;
    InputControlId Control;
    InputActionId Action;
};
}
