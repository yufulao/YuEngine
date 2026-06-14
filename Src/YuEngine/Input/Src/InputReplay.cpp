#include "YuEngine/Input/InputReplay.h"

namespace yuengine::input {
InputReplay::InputReplay()
    : bindings_{},
      frames_{},
      actions_{},
      registered_actions_{},
      snapshot_{},
      binding_count_(0U),
      recorded_frame_count_(0U),
      next_frame_index_(0U) {
    snapshot_.device_capacity = MAX_INPUT_DEVICES;
    snapshot_.action_capacity = MAX_INPUT_ACTIONS;
    snapshot_.binding_capacity = MAX_INPUT_BINDINGS;
    snapshot_.replay_frame_capacity = MAX_REPLAY_FRAMES;
    snapshot_.event_capacity_per_frame = MAX_EVENTS_PER_FRAME;
    snapshot_.replay_storage_capacity_before_frame = ReplayStorageCapacity();
    snapshot_.replay_storage_capacity_after_last_frame = ReplayStorageCapacity();
}

InputBindingResult InputReplay::RegisterActionBinding(InputDeviceId device, InputControlId control, InputActionId action) {
    if (!IsDeviceValid(device)) {
        return InputBindingResult{RecordFailure(InputStatus::UnknownDeviceControl), action};
    }

    if (!IsActionInRange(action)) {
        return InputBindingResult{RecordFailure(InputStatus::UnknownAction), action};
    }

    if (HasBindingForControl(device, control)) {
        return InputBindingResult{RecordFailure(InputStatus::DuplicateBinding), action};
    }

    if (binding_count_ >= bindings_.size()) {
        return InputBindingResult{RecordFailure(InputStatus::CapacityExceeded), action};
    }

    bindings_[binding_count_] = InputActionBinding{device, control, action};
    ++binding_count_;
    snapshot_.binding_count = binding_count_;

    if (!registered_actions_[action.value]) {
        registered_actions_[action.value] = true;
        ++snapshot_.action_count;
    }

    return InputBindingResult{InputStatus::Success, action};
}

InputStatus InputReplay::RecordReplayEvent(std::size_t frame_index, InputEvent event) {
    if (frame_index >= frames_.size()) {
        return RejectReplayEvent(InputStatus::CapacityExceeded);
    }

    if (!IsEventTypeKnown(event.type)) {
        return RejectReplayEvent(InputStatus::InvalidEvent);
    }

    if (event.type == InputEventType::Axis) {
        if (!IsAxisValueValid(event.axis_value)) {
            return RejectReplayEvent(InputStatus::InvalidAxisValue);
        }
    }

    if (FindBinding(event.device, event.control) == nullptr) {
        return RejectReplayEvent(InputStatus::UnknownDeviceControl);
    }

    InputReplayFrame& frame = frames_[frame_index];
    if (frame.event_count >= frame.events.size()) {
        return RejectReplayEvent(InputStatus::CapacityExceeded);
    }

    frame.events[frame.event_count] = event;
    ++frame.event_count;
    ++snapshot_.accepted_event_count;

    const std::size_t recorded_frame_count = frame_index + 1U;
    if (recorded_frame_count > recorded_frame_count_) {
        recorded_frame_count_ = recorded_frame_count;
    }

    return InputStatus::Success;
}

InputApplyResult InputReplay::ApplyNextFrame() {
    if (next_frame_index_ >= recorded_frame_count_) {
        snapshot_.last_apply_status = InputStatus::EndOfReplay;
        return InputApplyResult{RecordFailure(InputStatus::EndOfReplay), next_frame_index_};
    }

    ResetFrameState();
    snapshot_.replay_storage_capacity_before_frame = ReplayStorageCapacity();

    InputStatus frame_status = InputStatus::Success;
    const InputReplayFrame& frame = frames_[next_frame_index_];
    for (std::size_t event_index = 0U; event_index < frame.event_count; ++event_index) {
        const InputEvent& event = frame.events[event_index];
        const InputActionBinding* binding = FindBinding(event.device, event.control);
        if (binding == nullptr) {
            RejectReplayEvent(InputStatus::UnknownDeviceControl);
            frame_status = InputStatus::UnknownDeviceControl;
            continue;
        }

        InputActionState& action = actions_[binding->action.value];
        if (event.type == InputEventType::ButtonPressed) {
            action.is_pressed = true;
            MarkActionChanged(binding->action);
            continue;
        }

        if (event.type == InputEventType::ButtonReleased) {
            action.is_pressed = false;
            MarkActionChanged(binding->action);
            continue;
        }

        if (event.type == InputEventType::Axis) {
            action.axis_value = event.axis_value;
            MarkActionChanged(binding->action);
            continue;
        }

        RejectReplayEvent(InputStatus::InvalidEvent);
        frame_status = InputStatus::InvalidEvent;
    }

    RecalculateChangedActionCount();
    ++snapshot_.apply_count;
    snapshot_.last_apply_status = frame_status;
    snapshot_.replay_storage_capacity_after_last_frame = ReplayStorageCapacity();

    const std::size_t applied_frame_index = next_frame_index_;
    ++next_frame_index_;
    return InputApplyResult{frame_status, applied_frame_index};
}

InputStatus InputReplay::ResetFrameState() {
    for (InputActionState& action : actions_) {
        action.changed_this_frame = false;
    }

    snapshot_.changed_action_count = 0U;
    ++snapshot_.reset_count;
    return InputStatus::Success;
}

InputActionQueryResult InputReplay::QueryAction(InputActionId action) const {
    if (!IsActionInRange(action)) {
        return InputActionQueryResult{InputStatus::UnknownAction, InputActionState{}};
    }

    if (!registered_actions_[action.value]) {
        return InputActionQueryResult{InputStatus::UnknownAction, InputActionState{}};
    }

    return InputActionQueryResult{InputStatus::Success, actions_[action.value]};
}

InputReplaySnapshot InputReplay::Snapshot() const {
    return snapshot_;
}

std::size_t InputReplay::EventCountForFrame(std::size_t frame_index) const {
    if (frame_index >= frames_.size()) {
        return 0U;
    }

    return frames_[frame_index].event_count;
}

InputStatus InputReplay::RecordFailure(InputStatus status) {
    ++snapshot_.failed_operation_count;
    return status;
}

InputStatus InputReplay::RejectReplayEvent(InputStatus status) {
    ++snapshot_.rejected_event_count;
    return RecordFailure(status);
}

bool InputReplay::IsDeviceValid(InputDeviceId device) const {
    return device.value < MAX_INPUT_DEVICES;
}

bool InputReplay::IsActionInRange(InputActionId action) const {
    return action.value < MAX_INPUT_ACTIONS;
}

bool InputReplay::IsEventTypeKnown(InputEventType type) const {
    if (type == InputEventType::ButtonPressed) {
        return true;
    }

    if (type == InputEventType::ButtonReleased) {
        return true;
    }

    return type == InputEventType::Axis;
}

bool InputReplay::IsAxisValueValid(std::int32_t value) const {
    if (value < AXIS_MIN_VALUE) {
        return false;
    }

    return value <= AXIS_MAX_VALUE;
}

bool InputReplay::HasBindingForControl(InputDeviceId device, InputControlId control) const {
    return FindBinding(device, control) != nullptr;
}

const InputActionBinding* InputReplay::FindBinding(InputDeviceId device, InputControlId control) const {
    for (std::size_t index = 0U; index < binding_count_; ++index) {
        const InputActionBinding& binding = bindings_[index];
        if (binding.device.value != device.value) {
            continue;
        }

        if (binding.control.value != control.value) {
            continue;
        }

        return &binding;
    }

    return nullptr;
}

void InputReplay::MarkActionChanged(InputActionId action) {
    actions_[action.value].changed_this_frame = true;
}

void InputReplay::RecalculateChangedActionCount() {
    std::size_t changed_count = 0U;
    for (std::size_t index = 0U; index < actions_.size(); ++index) {
        if (!registered_actions_[index]) {
            continue;
        }

        if (!actions_[index].changed_this_frame) {
            continue;
        }

        ++changed_count;
    }

    snapshot_.changed_action_count = changed_count;
}

std::size_t InputReplay::ReplayStorageCapacity() const {
    return MAX_REPLAY_FRAMES * MAX_EVENTS_PER_FRAME;
}
}
