#pragma once

#include "YuEngine/Input/InputActionId.h"
#include "YuEngine/Input/InputControlId.h"
#include "YuEngine/Input/InputDeviceId.h"

namespace yuengine::input {
struct InputActionBinding final {
    InputDeviceId device;
    InputControlId control;
    InputActionId action;
};
}
