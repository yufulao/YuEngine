#pragma once

#include "yuengine/input/input_action_state.h"
#include "yuengine/input/input_status.h"

namespace yuengine::input {
struct InputActionQueryResult final {
    InputStatus Status = InputStatus::Success;
    InputActionState State;
};
}
