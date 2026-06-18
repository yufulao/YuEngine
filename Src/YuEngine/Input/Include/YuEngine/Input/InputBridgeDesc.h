// Module: YuEngine Input
// File: Src/YuEngine/Input/Include/YuEngine/Input/InputBridgeDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/Input/InputBackendKind.h"
#include "YuEngine/Input/InputDeviceId.h"

namespace yuengine::input {
enum class InputFocusPolicy {
    RejectWhenUnfocused,
    AcceptWhenUnfocused
};

struct InputBridgeDesc final {
    static constexpr std::size_t MIN_EVENT_CAPACITY = 1U;
    static constexpr std::size_t DEFAULT_EVENT_CAPACITY = 16U;
    static constexpr std::size_t MAX_EVENT_CAPACITY = 32U;

    InputBackendKind backend = InputBackendKind::NativeMessage;
    InputFocusPolicy focus_policy = InputFocusPolicy::RejectWhenUnfocused;
    std::size_t event_capacity = DEFAULT_EVENT_CAPACITY;
    InputDeviceId keyboard_device{0U};
    InputDeviceId mouse_device{1U};
    InputDeviceId gamepad_device{2U};
    bool start_focused = true;
};
}
