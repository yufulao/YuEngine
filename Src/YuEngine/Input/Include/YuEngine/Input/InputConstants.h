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
constexpr std::int32_t AXIS_MIN_VALUE = -32767;
constexpr std::int32_t AXIS_MAX_VALUE = 32767;
}
