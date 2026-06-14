#pragma once

#include "yuengine/input/InputActionId.h"
#include "yuengine/input/InputControlId.h"
#include "yuengine/input/InputDeviceId.h"

namespace yuengine::input {
struct InputActionBinding final {
    InputDeviceId Device;
    InputControlId Control;
    InputActionId Action;
};
}
