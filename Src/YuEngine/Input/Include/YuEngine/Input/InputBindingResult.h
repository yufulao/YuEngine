#pragma once

#include "YuEngine/Input/InputActionId.h"
#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
struct InputBindingResult final {
    InputStatus Status = InputStatus::Success;
    InputActionId Action;
};
}
