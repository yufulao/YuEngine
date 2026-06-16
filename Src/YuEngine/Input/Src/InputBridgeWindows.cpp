// Module: YuEngine Input
// File: Src/YuEngine/Input/Src/InputBridgeWindows.cpp

#include "YuEngine/Input/InputBridge.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace yuengine::input {
namespace {
constexpr std::uint32_t MOUSE_MOVE_CONTROL = 0U;
constexpr std::uint32_t MOUSE_WHEEL_CONTROL = 1U;
constexpr std::uint32_t MOUSE_LEFT_BUTTON = 2U;
constexpr std::uint32_t MOUSE_RIGHT_BUTTON = 3U;
constexpr std::uint32_t MOUSE_MIDDLE_BUTTON = 4U;

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
}
