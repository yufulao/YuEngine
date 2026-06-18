// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputEvent.h

#pragma once

#include <cstdint>

#include "YuEngine/Input/InputControlId.h"
#include "YuEngine/Input/InputDeviceId.h"
#include "YuEngine/Input/InputEventType.h"

namespace yuengine::input {
struct InputEvent final {
    InputDeviceId device;
    InputControlId control;
    InputEventType type = InputEventType::ButtonPressed;
    std::int32_t axis_value = 0;
};
}
