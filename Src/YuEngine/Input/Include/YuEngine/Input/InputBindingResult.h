#pragma once

#include "YuEngine/Input/InputActionId.h"
#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
struct InputBindingResult final {
    InputStatus status = InputStatus::Success;
    InputActionId action;
};
}
