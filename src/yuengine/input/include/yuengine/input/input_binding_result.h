#pragma once

#include "yuengine/input/input_action_id.h"
#include "yuengine/input/input_status.h"

namespace yuengine::input {
struct input_binding_result_t final {
    InputStatus Status = InputStatus::Success;
    input_action_id_t Action;
};
}
