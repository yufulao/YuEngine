#pragma once

#include "yuengine/input/input_action_id.h"
#include "yuengine/input/input_status.h"

namespace yuengine::input {
struct InputBindingResult final {
    InputStatus Status = InputStatus::Success;
    InputActionId Action;
};
}
