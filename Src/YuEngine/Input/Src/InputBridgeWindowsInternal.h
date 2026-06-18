// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Src/InputBridgeWindowsInternal.h

#pragma once

#include <cstdint>

namespace yuengine::input::internal {
enum class InputNativeGamepadPollStatus {
    Success,
    DeviceUnavailable,
    BackendError
};

struct InputNativeGamepadState final {
    std::uint32_t packet_number = 0U;
    std::uint16_t buttons = 0U;
    std::uint8_t left_trigger = 0U;
    std::uint8_t right_trigger = 0U;
    std::int32_t left_thumb_x = 0;
    std::int32_t left_thumb_y = 0;
    std::int32_t right_thumb_x = 0;
    std::int32_t right_thumb_y = 0;
};

using InputNativeGamepadPollFunction = InputNativeGamepadPollStatus (*)(std::uint32_t user_index, InputNativeGamepadState *state);

InputNativeGamepadPollFunction SetInputNativeGamepadPollFunctionForTest(InputNativeGamepadPollFunction function);
InputNativeGamepadPollStatus PollNativeXInputGamepad(std::uint32_t user_index, InputNativeGamepadState *state);
}
