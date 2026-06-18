// Module: YuEngine Input
// File: Src/YuEngine/Input/Include/YuEngine/Input/InputConstants.h

#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::input {
constexpr std::size_t MAX_INPUT_DEVICES = 4U;
constexpr std::size_t MAX_INPUT_ACTIONS = 32U;
constexpr std::size_t MAX_INPUT_BINDINGS = 64U;
constexpr std::size_t MAX_REPLAY_FRAMES = 16U;
constexpr std::size_t MAX_EVENTS_PER_FRAME = 32U;
constexpr std::uint32_t MAX_GAMEPAD_DEVICES = 4U;
constexpr std::int32_t AXIS_MIN_VALUE = -32767;
constexpr std::int32_t AXIS_MAX_VALUE = 32767;
constexpr std::uint16_t GAMEPAD_BUTTON_DPAD_UP = 0x0001U;
constexpr std::uint16_t GAMEPAD_BUTTON_DPAD_DOWN = 0x0002U;
constexpr std::uint16_t GAMEPAD_BUTTON_DPAD_LEFT = 0x0004U;
constexpr std::uint16_t GAMEPAD_BUTTON_DPAD_RIGHT = 0x0008U;
constexpr std::uint16_t GAMEPAD_BUTTON_START = 0x0010U;
constexpr std::uint16_t GAMEPAD_BUTTON_BACK = 0x0020U;
constexpr std::uint16_t GAMEPAD_BUTTON_LEFT_THUMB = 0x0040U;
constexpr std::uint16_t GAMEPAD_BUTTON_RIGHT_THUMB = 0x0080U;
constexpr std::uint16_t GAMEPAD_BUTTON_LEFT_SHOULDER = 0x0100U;
constexpr std::uint16_t GAMEPAD_BUTTON_RIGHT_SHOULDER = 0x0200U;
constexpr std::uint16_t GAMEPAD_BUTTON_A = 0x1000U;
constexpr std::uint16_t GAMEPAD_BUTTON_B = 0x2000U;
constexpr std::uint16_t GAMEPAD_BUTTON_X = 0x4000U;
constexpr std::uint16_t GAMEPAD_BUTTON_Y = 0x8000U;
constexpr std::uint32_t GAMEPAD_LEFT_TRIGGER_CONTROL = 0U;
constexpr std::uint32_t GAMEPAD_RIGHT_TRIGGER_CONTROL = 1U;
constexpr std::uint32_t GAMEPAD_LEFT_THUMB_X_CONTROL = 2U;
constexpr std::uint32_t GAMEPAD_LEFT_THUMB_Y_CONTROL = 3U;
constexpr std::uint32_t GAMEPAD_RIGHT_THUMB_X_CONTROL = 4U;
constexpr std::uint32_t GAMEPAD_RIGHT_THUMB_Y_CONTROL = 5U;
constexpr std::uint32_t GAMEPAD_BUTTON_CONTROL_BASE = 16U;
}
