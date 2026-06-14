#pragma once

#include "yuengine/input/input_action_state.h"
#include "yuengine/input/input_status.h"

namespace yuengine::input {
struct InputActionQueryResult final {
    INPUT_STATUS Status = INPUT_STATUS::Success;
    InputActionState State;
};
}
