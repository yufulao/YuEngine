// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputBridgeEvent.h

#pragma once

#include <cstdint>

#include "YuEngine/Input/InputControlId.h"
#include "YuEngine/Input/InputDeviceId.h"
#include "YuEngine/Input/InputDeviceKind.h"

namespace yuengine::input {
enum class InputBridgeEventType {
    None,
    KeyPressed,
    KeyReleased,
    MouseMoved,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseWheel,
    GamepadButtonPressed,
    GamepadButtonReleased,
    GamepadAxisMoved
};

struct InputBridgeEvent final {
    InputDeviceKind device_kind = InputDeviceKind::Unknown;
    InputDeviceId device;
    InputControlId control;
    InputBridgeEventType type = InputBridgeEventType::None;
    std::uint32_t raw_code = 0U;
    std::int32_t pointer_x = 0;
    std::int32_t pointer_y = 0;
    std::int32_t wheel_delta = 0;
    std::int32_t axis_value = 0;
};
}
