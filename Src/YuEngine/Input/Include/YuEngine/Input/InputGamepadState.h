// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputGamepadState.h

#pragma once

#include <cstdint>

#include "YuEngine/Input/InputDeviceId.h"
#include "YuEngine/Input/InputGamepadConnection.h"

namespace yuengine::input {

struct InputGamepadState final {
    InputDeviceId device{2U};
    InputGamepadConnection connection = InputGamepadConnection::Unavailable;
    std::uint32_t packet_number = 0U;
    std::uint16_t buttons = 0U;
    std::uint8_t left_trigger = 0U;
    std::uint8_t right_trigger = 0U;
    std::int32_t left_thumb_x = 0;
    std::int32_t left_thumb_y = 0;
    std::int32_t right_thumb_x = 0;
    std::int32_t right_thumb_y = 0;
};

}
