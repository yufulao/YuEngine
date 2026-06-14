#pragma once

#include "yuengine/input/input_action_id.h"
#include "yuengine/input/input_control_id.h"
#include "yuengine/input/input_device_id.h"

namespace yuengine::input {
struct input_action_binding_t final {
    input_device_id_t Device;
    input_control_id_t Control;
    input_action_id_t Action;
};
}
