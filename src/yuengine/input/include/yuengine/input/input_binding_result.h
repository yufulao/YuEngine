#pragma once

#include "yuengine/input/input_action_id.h"
#include "yuengine/input/input_status.h"

namespace yuengine::input {
struct InputBindingResult final {
    INPUT_STATUS Status = INPUT_STATUS::Success;
    InputActionId Action;
};
}
