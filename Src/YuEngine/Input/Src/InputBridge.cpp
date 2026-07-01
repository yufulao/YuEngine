// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Src/InputBridge.cpp

#include "YuEngine/Input/InputBridge.h"

#include "YuEngine/Input/InputConstants.h"

namespace yuengine::input {
namespace {
constexpr std::array<std::uint16_t, 14U> GAMEPAD_BUTTON_MASKS{
    GAMEPAD_BUTTON_DPAD_UP,
    GAMEPAD_BUTTON_DPAD_DOWN,
    GAMEPAD_BUTTON_DPAD_LEFT,
    GAMEPAD_BUTTON_DPAD_RIGHT,
    GAMEPAD_BUTTON_START,
    GAMEPAD_BUTTON_BACK,
    GAMEPAD_BUTTON_LEFT_THUMB,
    GAMEPAD_BUTTON_RIGHT_THUMB,
    GAMEPAD_BUTTON_LEFT_SHOULDER,
    GAMEPAD_BUTTON_RIGHT_SHOULDER,
    GAMEPAD_BUTTON_A,
    GAMEPAD_BUTTON_B,
    GAMEPAD_BUTTON_X,
    GAMEPAD_BUTTON_Y};

std::size_t CountAxisChange(std::int32_t previous_value, std::int32_t next_value) {
    if (previous_value == next_value) {
        return 0U;
    }

    return 1U;
}

std::size_t CountButtonChanges(std::uint16_t previous_buttons, std::uint16_t next_buttons) {
    std::size_t result = 0U;
    const std::uint16_t changed_buttons = static_cast<std::uint16_t>(previous_buttons ^ next_buttons);
    for (std::uint16_t button_mask : GAMEPAD_BUTTON_MASKS) {
        if ((changed_buttons & button_mask) != 0U) {
            ++result;
        }
    }

    return result;
}

InputBridgeEvent MakeGamepadButtonEvent(InputDeviceId device, std::uint32_t button_index, std::uint16_t button_mask, bool pressed) {
    InputBridgeEvent event{};
    event.device_kind = InputDeviceKind::Gamepad;
    event.device = device;
    event.control = InputControlId{GAMEPAD_BUTTON_CONTROL_BASE + button_index};
    event.type = InputBridgeEventType::GamepadButtonReleased;
    event.raw_code = button_mask;
    if (pressed) {
        event.type = InputBridgeEventType::GamepadButtonPressed;
    }

    return event;
}

InputBridgeEvent MakeGamepadAxisEvent(InputDeviceId device, std::uint32_t control, std::int32_t value) {
    InputBridgeEvent event{};
    event.device_kind = InputDeviceKind::Gamepad;
    event.device = device;
    event.control = InputControlId{control};
    event.type = InputBridgeEventType::GamepadAxisMoved;
    event.raw_code = control;
    event.axis_value = value;
    return event;
}
}

InputBridge::InputBridge()
    : events_{},
      desc_{},
      snapshot_{},
      gamepad_state_{},
      read_index_(0U),
      write_index_(0U),
      event_count_(0U),
      initialized_(false),
      focused_(false) {
}

InputBridge::~InputBridge() {
    static_cast<void>(Shutdown());
}

InputStatus InputBridge::Initialize(const InputBridgeDesc &desc) {
    if (initialized_) {
        return RecordStatus(InputStatus::AlreadyInitialized);
    }

    const InputStatus desc_status = ValidateDesc(desc);
    if (desc_status != InputStatus::Success) {
        return RecordStatus(desc_status);
    }

    desc_ = desc;
    snapshot_ = InputBridgeSnapshot{};
    snapshot_.backend = desc.backend;
    snapshot_.event_capacity = desc.event_capacity;
    snapshot_.initialized = true;
    snapshot_.focused = desc.start_focused;
    snapshot_.gamepad_connection = InputGamepadConnection::Unavailable;
    snapshot_.last_status = InputStatus::Success;
    gamepad_state_ = InputGamepadState{};
    gamepad_state_.device = desc.gamepad_device;
    initialized_ = true;
    focused_ = desc.start_focused;
    ClearQueuedEvents();
    return InputStatus::Success;
}

InputStatus InputBridge::Shutdown() {
    if (!initialized_) {
        snapshot_.last_status = InputStatus::Success;
        return InputStatus::Success;
    }

    ClearQueuedEvents();
    initialized_ = false;
    focused_ = false;
    snapshot_.initialized = false;
    snapshot_.focused = false;
    snapshot_.gamepad_connection = InputGamepadConnection::Unavailable;
    ClearGamepadCapacityFailure();
    snapshot_.last_status = InputStatus::Success;
    gamepad_state_ = InputGamepadState{};
    return InputStatus::Success;
}

InputStatus InputBridge::SetFocus(bool focused) {
    if (!initialized_) {
        return RecordStatus(InputStatus::NotInitialized);
    }

    if (focused_ == focused) {
        snapshot_.focused = focused_;
        return RecordStatus(InputStatus::Success);
    }

    focused_ = focused;
    snapshot_.focused = focused_;
    if (focused_) {
        ++snapshot_.focus_gained_count;
        return RecordStatus(InputStatus::Success);
    }

    ++snapshot_.focus_lost_count;
    return RecordStatus(InputStatus::Success);
}

InputStatus InputBridge::SubmitEvent(const InputBridgeEvent &event) {
    if (!initialized_) {
        return RejectEvent(InputStatus::NotInitialized);
    }

    const InputStatus event_status = ValidateEvent(event);
    if (event_status != InputStatus::Success) {
        return RejectEvent(event_status);
    }

    if (!focused_) {
        if (desc_.focus_policy == InputFocusPolicy::RejectWhenUnfocused) {
            return RejectEvent(InputStatus::FocusLost);
        }
    }

    return AcceptEvent(event);
}

InputStatus InputBridge::SubmitGamepadState(const InputGamepadState &state) {
    if (!initialized_) {
        return RejectEvent(InputStatus::NotInitialized);
    }

    if (!IsDeviceValid(state.device)) {
        return RejectEvent(InputStatus::UnknownDeviceControl);
    }

    if (state.device.value != desc_.gamepad_device.value) {
        return RejectEvent(InputStatus::UnknownDeviceControl);
    }

    if (state.connection == InputGamepadConnection::Unknown) {
        return RejectEvent(InputStatus::InvalidEvent);
    }

    ++snapshot_.gamepad_poll_count;
    if (state.connection == InputGamepadConnection::Unavailable) {
        ++snapshot_.gamepad_unavailable_poll_count;
        snapshot_.gamepad_connection = InputGamepadConnection::Unavailable;
        gamepad_state_ = InputGamepadState{};
        gamepad_state_.device = desc_.gamepad_device;
        return RecordStatus(InputStatus::DeviceUnavailable);
    }

    if (state.connection != InputGamepadConnection::Connected) {
        return RejectEvent(InputStatus::InvalidEvent);
    }

    return AcceptGamepadState(state);
}

InputStatus InputBridge::DrainEvents(InputBridgeEvent *events, std::size_t event_capacity, std::size_t &out_event_count) {
    out_event_count = 0U;

    if (!initialized_) {
        return RecordStatus(InputStatus::NotInitialized);
    }

    if (events == nullptr) {
        if (event_count_ > 0U) {
            return RecordStatus(InputStatus::NullPointer);
        }
    }

    if (event_capacity < event_count_) {
        snapshot_.required_output_event_count = event_count_;
        return RecordStatus(InputStatus::OutputBufferFull);
    }

    const std::size_t drain_count = event_count_;
    for (std::size_t index = 0U; index < drain_count; ++index) {
        events[index] = events_[read_index_];
        read_index_ = (read_index_ + 1U) % desc_.event_capacity;
    }

    event_count_ = 0U;
    write_index_ = 0U;
    read_index_ = 0U;
    out_event_count = drain_count;
    snapshot_.queued_event_count = event_count_;
    snapshot_.required_output_event_count = event_count_;
    snapshot_.drained_event_count += drain_count;
    return RecordStatus(InputStatus::Success);
}

InputBridgeSnapshot InputBridge::Snapshot() const {
    return snapshot_;
}

InputStatus InputBridge::ValidateDesc(const InputBridgeDesc &desc) const {
    if (desc.backend == InputBackendKind::Replay) {
        return InputStatus::UnsupportedBackend;
    }

    if (desc.backend != InputBackendKind::NativeMessage) {
        return InputStatus::UnsupportedBackend;
    }

    if (desc.event_capacity < InputBridgeDesc::MIN_EVENT_CAPACITY) {
        return InputStatus::InvalidDescriptor;
    }

    if (desc.event_capacity > InputBridgeDesc::MAX_EVENT_CAPACITY) {
        return InputStatus::InvalidDescriptor;
    }

    if (desc.keyboard_device.value >= MAX_INPUT_DEVICES) {
        return InputStatus::UnknownDeviceControl;
    }

    if (desc.mouse_device.value >= MAX_INPUT_DEVICES) {
        return InputStatus::UnknownDeviceControl;
    }

    if (desc.gamepad_device.value >= MAX_INPUT_DEVICES) {
        return InputStatus::UnknownDeviceControl;
    }

    return InputStatus::Success;
}

InputStatus InputBridge::ValidateEvent(const InputBridgeEvent &event) const {
    if (!IsEventKnown(event.type)) {
        return InputStatus::InvalidEvent;
    }

    if (!IsDeviceValid(event.device)) {
        return InputStatus::UnknownDeviceControl;
    }

    if (event.device_kind == InputDeviceKind::Unknown) {
        return InputStatus::InvalidEvent;
    }

    if (event.device_kind == InputDeviceKind::Gamepad) {
        if (event.type == InputBridgeEventType::GamepadButtonPressed) {
            if (event.raw_code == 0U) {
                return InputStatus::UnknownDeviceControl;
            }

            return InputStatus::Success;
        }

        if (event.type == InputBridgeEventType::GamepadButtonReleased) {
            if (event.raw_code == 0U) {
                return InputStatus::UnknownDeviceControl;
            }

            return InputStatus::Success;
        }

        if (event.type == InputBridgeEventType::GamepadAxisMoved) {
            if (!IsAxisValueValid(event.axis_value)) {
                return InputStatus::InvalidAxisValue;
            }

            return InputStatus::Success;
        }

        return InputStatus::InvalidEvent;
    }

    if (event.type == InputBridgeEventType::KeyPressed) {
        if (event.device_kind != InputDeviceKind::Keyboard) {
            return InputStatus::InvalidEvent;
        }

        if (event.raw_code == 0U) {
            return InputStatus::UnknownDeviceControl;
        }

        return InputStatus::Success;
    }

    if (event.type == InputBridgeEventType::KeyReleased) {
        if (event.device_kind != InputDeviceKind::Keyboard) {
            return InputStatus::InvalidEvent;
        }

        if (event.raw_code == 0U) {
            return InputStatus::UnknownDeviceControl;
        }

        return InputStatus::Success;
    }

    if (event.device_kind != InputDeviceKind::Mouse) {
        return InputStatus::InvalidEvent;
    }

    if (event.type == InputBridgeEventType::MouseMoved) {
        if (!IsAxisValueValid(event.pointer_x)) {
            return InputStatus::InvalidAxisValue;
        }

        if (!IsAxisValueValid(event.pointer_y)) {
            return InputStatus::InvalidAxisValue;
        }

        return InputStatus::Success;
    }

    if (event.type == InputBridgeEventType::MouseWheel) {
        if (!IsAxisValueValid(event.wheel_delta)) {
            return InputStatus::InvalidAxisValue;
        }

        return InputStatus::Success;
    }

    if (event.raw_code == 0U) {
        return InputStatus::UnknownDeviceControl;
    }

    return InputStatus::Success;
}

InputStatus InputBridge::RecordStatus(InputStatus status) {
    ClearGamepadCapacityFailure();
    snapshot_.last_status = status;
    if (status == InputStatus::Success) {
        return status;
    }

    ++snapshot_.failed_operation_count;
    if (status == InputStatus::UnsupportedBackend) {
        ++snapshot_.unsupported_backend_count;
        return status;
    }

    if (status == InputStatus::DeviceUnavailable) {
        ++snapshot_.unavailable_count;
        return status;
    }

    if (status == InputStatus::CapacityExceeded) {
        ++snapshot_.overflow_count;
        return status;
    }

    return status;
}

InputStatus InputBridge::RejectEvent(InputStatus status) {
    ++snapshot_.rejected_event_count;
    return RecordStatus(status);
}

InputStatus InputBridge::RejectGamepadCapacity(
    const InputGamepadState &state,
    std::size_t event_capacity,
    std::size_t event_count,
    std::size_t required_event_count) {
    ++snapshot_.rejected_event_count;
    ++snapshot_.failed_operation_count;
    ++snapshot_.overflow_count;
    snapshot_.last_status = InputStatus::CapacityExceeded;
    snapshot_.last_failed_gamepad_event_capacity = event_capacity;
    snapshot_.last_failed_gamepad_event_count = event_count;
    snapshot_.last_required_gamepad_event_count = required_event_count;
    snapshot_.last_failed_gamepad_device = state.device;
    snapshot_.last_failed_gamepad_connection = state.connection;
    snapshot_.last_failed_gamepad_packet_number = state.packet_number;
    snapshot_.last_failed_gamepad_button_bits = state.buttons;
    snapshot_.last_failed_gamepad_left_trigger = state.left_trigger;
    snapshot_.last_failed_gamepad_right_trigger = state.right_trigger;
    snapshot_.last_failed_gamepad_left_thumb_x = state.left_thumb_x;
    snapshot_.last_failed_gamepad_left_thumb_y = state.left_thumb_y;
    snapshot_.last_failed_gamepad_right_thumb_x = state.right_thumb_x;
    snapshot_.last_failed_gamepad_right_thumb_y = state.right_thumb_y;
    return InputStatus::CapacityExceeded;
}

InputStatus InputBridge::AcceptEvent(const InputBridgeEvent &event) {
    if (event_count_ >= desc_.event_capacity) {
        return RejectEvent(InputStatus::CapacityExceeded);
    }

    events_[write_index_] = event;
    write_index_ = (write_index_ + 1U) % desc_.event_capacity;
    ++event_count_;
    ++snapshot_.accepted_event_count;
    snapshot_.queued_event_count = event_count_;
    snapshot_.required_output_event_count = event_count_;
    if (snapshot_.queued_event_count > snapshot_.max_queued_event_count) {
        snapshot_.max_queued_event_count = snapshot_.queued_event_count;
    }

    return RecordStatus(InputStatus::Success);
}

InputStatus InputBridge::AcceptGamepadState(const InputGamepadState &state) {
    if (!IsAxisValueValid(state.left_thumb_x)) {
        return RejectEvent(InputStatus::InvalidAxisValue);
    }

    if (!IsAxisValueValid(state.left_thumb_y)) {
        return RejectEvent(InputStatus::InvalidAxisValue);
    }

    if (!IsAxisValueValid(state.right_thumb_x)) {
        return RejectEvent(InputStatus::InvalidAxisValue);
    }

    if (!IsAxisValueValid(state.right_thumb_y)) {
        return RejectEvent(InputStatus::InvalidAxisValue);
    }

    if (!focused_) {
        if (desc_.focus_policy == InputFocusPolicy::RejectWhenUnfocused) {
            return RejectEvent(InputStatus::FocusLost);
        }
    }

    ++snapshot_.gamepad_connected_poll_count;
    snapshot_.gamepad_connection = InputGamepadConnection::Connected;
    snapshot_.last_gamepad_packet_number = state.packet_number;
    if (gamepad_state_.connection == InputGamepadConnection::Connected) {
        if (state.packet_number == gamepad_state_.packet_number) {
            gamepad_state_ = state;
            return RecordStatus(InputStatus::Success);
        }
    }

    const std::size_t state_event_count = CountGamepadStateEvents(state);
    const std::size_t event_count = event_count_;
    const std::size_t required_event_count = event_count + state_event_count;
    if (required_event_count > desc_.event_capacity) {
        return RejectGamepadCapacity(state, desc_.event_capacity, event_count, required_event_count);
    }

    SubmitGamepadStateEvents(state);
    gamepad_state_ = state;
    snapshot_.gamepad_event_count += state_event_count;
    return RecordStatus(InputStatus::Success);
}

void InputBridge::ClearGamepadCapacityFailure() {
    snapshot_.last_failed_gamepad_event_capacity = 0U;
    snapshot_.last_failed_gamepad_event_count = 0U;
    snapshot_.last_required_gamepad_event_count = 0U;
    snapshot_.last_failed_gamepad_device = InputDeviceId{};
    snapshot_.last_failed_gamepad_connection = InputGamepadConnection::Unavailable;
    snapshot_.last_failed_gamepad_packet_number = 0U;
    snapshot_.last_failed_gamepad_button_bits = 0U;
    snapshot_.last_failed_gamepad_left_trigger = 0U;
    snapshot_.last_failed_gamepad_right_trigger = 0U;
    snapshot_.last_failed_gamepad_left_thumb_x = 0;
    snapshot_.last_failed_gamepad_left_thumb_y = 0;
    snapshot_.last_failed_gamepad_right_thumb_x = 0;
    snapshot_.last_failed_gamepad_right_thumb_y = 0;
}

void InputBridge::ClearQueuedEvents() {
    for (InputBridgeEvent &event : events_) {
        event = InputBridgeEvent{};
    }

    read_index_ = 0U;
    write_index_ = 0U;
    event_count_ = 0U;
    snapshot_.queued_event_count = 0U;
    snapshot_.required_output_event_count = 0U;
}

std::size_t InputBridge::CountGamepadStateEvents(const InputGamepadState &state) const {
    std::size_t result = CountButtonChanges(gamepad_state_.buttons, state.buttons);
    result += CountAxisChange(gamepad_state_.left_trigger, state.left_trigger);
    result += CountAxisChange(gamepad_state_.right_trigger, state.right_trigger);
    result += CountAxisChange(gamepad_state_.left_thumb_x, state.left_thumb_x);
    result += CountAxisChange(gamepad_state_.left_thumb_y, state.left_thumb_y);
    result += CountAxisChange(gamepad_state_.right_thumb_x, state.right_thumb_x);
    result += CountAxisChange(gamepad_state_.right_thumb_y, state.right_thumb_y);
    return result;
}

void InputBridge::SubmitGamepadStateEvents(const InputGamepadState &state) {
    const std::uint16_t changed_buttons = static_cast<std::uint16_t>(gamepad_state_.buttons ^ state.buttons);
    for (std::size_t index = 0U; index < GAMEPAD_BUTTON_MASKS.size(); ++index) {
        const std::uint16_t button_mask = GAMEPAD_BUTTON_MASKS[index];
        if ((changed_buttons & button_mask) == 0U) {
            continue;
        }

        const bool pressed = (state.buttons & button_mask) != 0U;
        const auto button_index = static_cast<std::uint32_t>(index);
        const InputBridgeEvent event = MakeGamepadButtonEvent(state.device, button_index, button_mask, pressed);
        static_cast<void>(AcceptEvent(event));
    }

    if (gamepad_state_.left_trigger != state.left_trigger) {
        const InputBridgeEvent event = MakeGamepadAxisEvent(state.device, GAMEPAD_LEFT_TRIGGER_CONTROL, state.left_trigger);
        static_cast<void>(AcceptEvent(event));
    }

    if (gamepad_state_.right_trigger != state.right_trigger) {
        const InputBridgeEvent event = MakeGamepadAxisEvent(state.device, GAMEPAD_RIGHT_TRIGGER_CONTROL, state.right_trigger);
        static_cast<void>(AcceptEvent(event));
    }

    if (gamepad_state_.left_thumb_x != state.left_thumb_x) {
        const InputBridgeEvent event = MakeGamepadAxisEvent(state.device, GAMEPAD_LEFT_THUMB_X_CONTROL, state.left_thumb_x);
        static_cast<void>(AcceptEvent(event));
    }

    if (gamepad_state_.left_thumb_y != state.left_thumb_y) {
        const InputBridgeEvent event = MakeGamepadAxisEvent(state.device, GAMEPAD_LEFT_THUMB_Y_CONTROL, state.left_thumb_y);
        static_cast<void>(AcceptEvent(event));
    }

    if (gamepad_state_.right_thumb_x != state.right_thumb_x) {
        const InputBridgeEvent event = MakeGamepadAxisEvent(state.device, GAMEPAD_RIGHT_THUMB_X_CONTROL, state.right_thumb_x);
        static_cast<void>(AcceptEvent(event));
    }

    if (gamepad_state_.right_thumb_y != state.right_thumb_y) {
        const InputBridgeEvent event = MakeGamepadAxisEvent(state.device, GAMEPAD_RIGHT_THUMB_Y_CONTROL, state.right_thumb_y);
        static_cast<void>(AcceptEvent(event));
    }
}

bool InputBridge::IsDeviceValid(InputDeviceId device) const {
    return device.value < MAX_INPUT_DEVICES;
}

bool InputBridge::IsEventKnown(InputBridgeEventType type) const {
    if (type == InputBridgeEventType::KeyPressed) {
        return true;
    }

    if (type == InputBridgeEventType::KeyReleased) {
        return true;
    }

    if (type == InputBridgeEventType::MouseMoved) {
        return true;
    }

    if (type == InputBridgeEventType::MouseButtonPressed) {
        return true;
    }

    if (type == InputBridgeEventType::MouseButtonReleased) {
        return true;
    }

    if (type == InputBridgeEventType::MouseWheel) {
        return true;
    }

    if (type == InputBridgeEventType::GamepadButtonPressed) {
        return true;
    }

    if (type == InputBridgeEventType::GamepadButtonReleased) {
        return true;
    }

    return type == InputBridgeEventType::GamepadAxisMoved;
}

bool InputBridge::IsAxisValueValid(std::int32_t value) const {
    if (value < AXIS_MIN_VALUE) {
        return false;
    }

    return value <= AXIS_MAX_VALUE;
}
}
