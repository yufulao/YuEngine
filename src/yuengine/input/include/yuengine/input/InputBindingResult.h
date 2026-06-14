#pragma once

#include "yuengine/input/InputActionId.h"
#include "yuengine/input/InputStatus.h"

namespace yuengine::input {
struct InputBindingResult final {
    InputStatus Status = InputStatus::Success;
    InputActionId Action;
};
}
