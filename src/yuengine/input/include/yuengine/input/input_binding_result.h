#pragma once

#include "yuengine/input/input_action_id.h"
#include "yuengine/input/input_status.h"

namespace yuengine::input {
struct input_binding_result_t final {
    INPUT_STATUS Status = INPUT_STATUS::Success;
    input_action_id_t Action;
};
}
