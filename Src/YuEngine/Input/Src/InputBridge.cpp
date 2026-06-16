// Module: YuEngine Input
// File: Src/YuEngine/Input/Src/InputBridge.cpp

#include "YuEngine/Input/InputBridge.h"

#include "YuEngine/Input/InputConstants.h"

namespace yuengine::input {
InputBridge::InputBridge()
    : events_{},
      desc_{},
      snapshot_{},
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
    snapshot_.last_status = InputStatus::Success;
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
    snapshot_.last_status = InputStatus::Success;
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

    return InputStatus::Success;
}

InputStatus InputBridge::ValidateEvent(const InputBridgeEvent &event) const {
    if (!IsEventKnown(event.type)) {
        return InputStatus::InvalidEvent;
    }

    if (!IsDeviceValid(event.device)) {
        return InputStatus::UnknownDeviceControl;
    }

    if (event.device_kind == InputDeviceKind::Gamepad) {
        return InputStatus::SourceUnavailable;
    }

    if (event.device_kind == InputDeviceKind::Unknown) {
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
    snapshot_.last_status = status;
    if (status == InputStatus::Success) {
        return status;
    }

    ++snapshot_.failed_operation_count;
    if (status == InputStatus::UnsupportedBackend) {
        ++snapshot_.unsupported_backend_count;
        return status;
    }

    if (status == InputStatus::SourceUnavailable) {
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

InputStatus InputBridge::AcceptEvent(const InputBridgeEvent &event) {
    if (event_count_ >= desc_.event_capacity) {
        return RejectEvent(InputStatus::CapacityExceeded);
    }

    events_[write_index_] = event;
    write_index_ = (write_index_ + 1U) % desc_.event_capacity;
    ++event_count_;
    ++snapshot_.accepted_event_count;
    snapshot_.queued_event_count = event_count_;
    if (snapshot_.queued_event_count > snapshot_.max_queued_event_count) {
        snapshot_.max_queued_event_count = snapshot_.queued_event_count;
    }

    return RecordStatus(InputStatus::Success);
}

void InputBridge::ClearQueuedEvents() {
    for (InputBridgeEvent &event : events_) {
        event = InputBridgeEvent{};
    }

    read_index_ = 0U;
    write_index_ = 0U;
    event_count_ = 0U;
    snapshot_.queued_event_count = 0U;
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

    return type == InputBridgeEventType::MouseWheel;
}

bool InputBridge::IsAxisValueValid(std::int32_t value) const {
    if (value < AXIS_MIN_VALUE) {
        return false;
    }

    return value <= AXIS_MAX_VALUE;
}
}
