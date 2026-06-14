#pragma once

#include "yuengine/input/input_action_state.h"
#include "yuengine/input/input_status.h"

namespace yuengine::input {
struct input_action_query_result_t final {
    INPUT_STATUS Status = INPUT_STATUS::Success;
    input_action_state_t State;
};
}
