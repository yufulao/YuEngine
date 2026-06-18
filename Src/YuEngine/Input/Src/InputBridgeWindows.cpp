// Module: YuEngine Input
// File: Src/YuEngine/Input/Src/InputBridgeWindows.cpp

#include "YuEngine/Input/InputBridge.h"

#include "InputBridgeWindowsInternal.h"

#include "YuEngine/Input/InputConstants.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <Xinput.h>

namespace yuengine::input {
namespace {
constexpr std::uint32_t MOUSE_MOVE_CONTROL = 0U;
constexpr std::uint32_t MOUSE_WHEEL_CONTROL = 1U;
constexpr std::uint32_t MOUSE_LEFT_BUTTON = 2U;
constexpr std::uint32_t MOUSE_RIGHT_BUTTON = 3U;
constexpr std::uint32_t MOUSE_MIDDLE_BUTTON = 4U;

internal::InputNativeGamepadPollFunction g_input_native_gamepad_poll_function = nullptr;

std::int32_t ReadSignedLowWord(std::intptr_t value) {
    const std::uint16_t word_value = static_cast<std::uint16_t>(value & 0xFFFF);
    return static_cast<std::int16_t>(word_value);
}

std::int32_t ReadSignedHighWord(std::intptr_t value) {
    const std::uint16_t word_value = static_cast<std::uint16_t>((value >> 16) & 0xFFFF);
    return static_cast<std::int16_t>(word_value);
}

InputBridgeEvent MakeKeyEvent(InputDeviceId device, std::uint32_t raw_code, InputBridgeEventType type) {
    InputBridgeEvent event{};
    event.device_kind = InputDeviceKind::Keyboard;
    event.device = device;
    event.control = InputControlId{raw_code};
    event.type = type;
    event.raw_code = raw_code;
    return event;
}

InputBridgeEvent MakeMouseButtonEvent(InputDeviceId device, std::uint32_t raw_code, std::intptr_t long_value, InputBridgeEventType type) {
    InputBridgeEvent event{};
    event.device_kind = InputDeviceKind::Mouse;
    event.device = device;
    event.control = InputControlId{raw_code};
    event.type = type;
    event.raw_code = raw_code;
    event.pointer_x = ReadSignedLowWord(long_value);
    event.pointer_y = ReadSignedHighWord(long_value);
    return event;
}

InputBridgeEvent MakeMouseMoveEvent(InputDeviceId device, std::intptr_t long_value) {
    InputBridgeEvent event{};
    event.device_kind = InputDeviceKind::Mouse;
    event.device = device;
    event.control = InputControlId{MOUSE_MOVE_CONTROL};
    event.type = InputBridgeEventType::MouseMoved;
    event.pointer_x = ReadSignedLowWord(long_value);
    event.pointer_y = ReadSignedHighWord(long_value);
    return event;
}

InputBridgeEvent MakeMouseWheelEvent(InputDeviceId device, std::uintptr_t word_value, std::intptr_t long_value) {
    InputBridgeEvent event{};
    event.device_kind = InputDeviceKind::Mouse;
    event.device = device;
    event.control = InputControlId{MOUSE_WHEEL_CONTROL};
    event.type = InputBridgeEventType::MouseWheel;
    event.pointer_x = ReadSignedLowWord(long_value);
    event.pointer_y = ReadSignedHighWord(long_value);
    event.wheel_delta = ReadSignedHighWord(static_cast<std::intptr_t>(word_value));
    event.axis_value = event.wheel_delta;
    return event;
}

std::int32_t ClampGamepadAxis(std::int32_t value) {
    if (value < AXIS_MIN_VALUE) {
        return AXIS_MIN_VALUE;
    }

    if (value > AXIS_MAX_VALUE) {
        return AXIS_MAX_VALUE;
    }

    return value;
}

InputGamepadState MakeUnavailableGamepadState(InputDeviceId device) {
    InputGamepadState state{};
    state.device = device;
    state.connection = InputGamepadConnection::Unavailable;
    return state;
}

InputGamepadState MakeConnectedGamepadState(InputDeviceId device, const internal::InputNativeGamepadState &native_state) {
    InputGamepadState state{};
    state.device = device;
    state.connection = InputGamepadConnection::Connected;
    state.packet_number = native_state.packet_number;
    state.buttons = native_state.buttons;
    state.left_trigger = native_state.left_trigger;
    state.right_trigger = native_state.right_trigger;
    state.left_thumb_x = ClampGamepadAxis(native_state.left_thumb_x);
    state.left_thumb_y = ClampGamepadAxis(native_state.left_thumb_y);
    state.right_thumb_x = ClampGamepadAxis(native_state.right_thumb_x);
    state.right_thumb_y = ClampGamepadAxis(native_state.right_thumb_y);
    return state;
}

internal::InputNativeGamepadPollFunction GetNativeGamepadPollFunction() {
    if (g_input_native_gamepad_poll_function == nullptr) {
        return internal::PollNativeXInputGamepad;
    }

    return g_input_native_gamepad_poll_function;
}
}

namespace internal {
InputNativeGamepadPollFunction SetInputNativeGamepadPollFunctionForTest(InputNativeGamepadPollFunction function) {
    const InputNativeGamepadPollFunction previous_function = g_input_native_gamepad_poll_function;
    g_input_native_gamepad_poll_function = function;
    return previous_function;
}

InputNativeGamepadPollStatus PollNativeXInputGamepad(std::uint32_t user_index, InputNativeGamepadState *state) {
    if (state == nullptr) {
        return InputNativeGamepadPollStatus::BackendError;
    }

    XINPUT_STATE native_state{};
    const DWORD poll_result = XInputGetState(user_index, &native_state);
    if (poll_result == ERROR_DEVICE_NOT_CONNECTED) {
        *state = InputNativeGamepadState{};
        return InputNativeGamepadPollStatus::DeviceUnavailable;
    }

    if (poll_result != ERROR_SUCCESS) {
        *state = InputNativeGamepadState{};
        return InputNativeGamepadPollStatus::BackendError;
    }

    state->packet_number = native_state.dwPacketNumber;
    state->buttons = native_state.Gamepad.wButtons;
    state->left_trigger = native_state.Gamepad.bLeftTrigger;
    state->right_trigger = native_state.Gamepad.bRightTrigger;
    state->left_thumb_x = ClampGamepadAxis(native_state.Gamepad.sThumbLX);
    state->left_thumb_y = ClampGamepadAxis(native_state.Gamepad.sThumbLY);
    state->right_thumb_x = ClampGamepadAxis(native_state.Gamepad.sThumbRX);
    state->right_thumb_y = ClampGamepadAxis(native_state.Gamepad.sThumbRY);
    return InputNativeGamepadPollStatus::Success;
}
}

InputStatus InputBridge::SubmitSourceMessage(std::uint32_t message_code, std::uintptr_t word_value, std::intptr_t long_value, bool source_focused) {
    if (!initialized_) {
        return RejectEvent(InputStatus::NotInitialized);
    }

    if (source_focused != focused_) {
        const InputStatus focus_status = SetFocus(source_focused);
        if (focus_status != InputStatus::Success) {
            return focus_status;
        }
    }

    switch (message_code) {
        case WM_SETFOCUS:
        {
            return SetFocus(true);
        }
        case WM_KILLFOCUS:
        {
            return SetFocus(false);
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            const std::uint32_t raw_code = static_cast<std::uint32_t>(word_value);
            const InputBridgeEvent event = MakeKeyEvent(desc_.keyboard_device, raw_code, InputBridgeEventType::KeyPressed);
            return SubmitEvent(event);
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            const std::uint32_t raw_code = static_cast<std::uint32_t>(word_value);
            const InputBridgeEvent event = MakeKeyEvent(desc_.keyboard_device, raw_code, InputBridgeEventType::KeyReleased);
            return SubmitEvent(event);
        }
        case WM_MOUSEMOVE:
        {
            const InputBridgeEvent event = MakeMouseMoveEvent(desc_.mouse_device, long_value);
            return SubmitEvent(event);
        }
        case WM_LBUTTONDOWN:
        {
            const InputBridgeEvent event = MakeMouseButtonEvent(desc_.mouse_device, MOUSE_LEFT_BUTTON, long_value, InputBridgeEventType::MouseButtonPressed);
            return SubmitEvent(event);
        }
        case WM_LBUTTONUP:
        {
            const InputBridgeEvent event = MakeMouseButtonEvent(desc_.mouse_device, MOUSE_LEFT_BUTTON, long_value, InputBridgeEventType::MouseButtonReleased);
            return SubmitEvent(event);
        }
        case WM_RBUTTONDOWN:
        {
            const InputBridgeEvent event = MakeMouseButtonEvent(desc_.mouse_device, MOUSE_RIGHT_BUTTON, long_value, InputBridgeEventType::MouseButtonPressed);
            return SubmitEvent(event);
        }
        case WM_RBUTTONUP:
        {
            const InputBridgeEvent event = MakeMouseButtonEvent(desc_.mouse_device, MOUSE_RIGHT_BUTTON, long_value, InputBridgeEventType::MouseButtonReleased);
            return SubmitEvent(event);
        }
        case WM_MBUTTONDOWN:
        {
            const InputBridgeEvent event = MakeMouseButtonEvent(desc_.mouse_device, MOUSE_MIDDLE_BUTTON, long_value, InputBridgeEventType::MouseButtonPressed);
            return SubmitEvent(event);
        }
        case WM_MBUTTONUP:
        {
            const InputBridgeEvent event = MakeMouseButtonEvent(desc_.mouse_device, MOUSE_MIDDLE_BUTTON, long_value, InputBridgeEventType::MouseButtonReleased);
            return SubmitEvent(event);
        }
        case WM_MOUSEWHEEL:
        {
            const InputBridgeEvent event = MakeMouseWheelEvent(desc_.mouse_device, word_value, long_value);
            return SubmitEvent(event);
        }
        default:
        {
            return RejectEvent(InputStatus::SourceUnavailable);
        }
    }
}

InputStatus InputBridge::PollGamepad(std::uint32_t user_index) {
    if (!initialized_) {
        return RejectEvent(InputStatus::NotInitialized);
    }

    if (user_index >= MAX_GAMEPAD_DEVICES) {
        return RejectEvent(InputStatus::InvalidUserIndex);
    }

    internal::InputNativeGamepadState native_state{};
    const internal::InputNativeGamepadPollFunction poll_function = GetNativeGamepadPollFunction();
    const internal::InputNativeGamepadPollStatus poll_status = poll_function(user_index, &native_state);
    if (poll_status == internal::InputNativeGamepadPollStatus::DeviceUnavailable) {
        const InputGamepadState state = MakeUnavailableGamepadState(desc_.gamepad_device);
        return SubmitGamepadState(state);
    }

    if (poll_status == internal::InputNativeGamepadPollStatus::BackendError) {
        return RejectEvent(InputStatus::BackendError);
    }

    if (poll_status != internal::InputNativeGamepadPollStatus::Success) {
        return RejectEvent(InputStatus::BackendError);
    }

    const InputGamepadState state = MakeConnectedGamepadState(desc_.gamepad_device, native_state);
    return SubmitGamepadState(state);
}
}
