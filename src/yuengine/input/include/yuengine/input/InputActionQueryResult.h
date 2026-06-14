#pragma once

#include "yuengine/input/InputActionState.h"
#include "yuengine/input/InputStatus.h"

namespace yuengine::input {
struct InputActionQueryResult final {
    InputStatus Status = InputStatus::Success;
    InputActionState State;
};
}
