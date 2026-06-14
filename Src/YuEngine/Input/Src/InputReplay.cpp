#include "YuEngine/Input/InputReplay.h"

namespace yuengine::input {
InputReplay::InputReplay()
    : _bindings{},
      _frames{},
      _actions{},
      _registeredActions{},
      _snapshot{},
      _bindingCount(0U),
      _recordedFrameCount(0U),
      _nextFrameIndex(0U) {
    _snapshot.device_capacity = MAX_INPUT_DEVICES;
    _snapshot.action_capacity = MAX_INPUT_ACTIONS;
    _snapshot.binding_capacity = MAX_INPUT_BINDINGS;
    _snapshot.replay_frame_capacity = MAX_REPLAY_FRAMES;
    _snapshot.event_capacity_per_frame = MAX_EVENTS_PER_FRAME;
    _snapshot.replay_storage_capacity_before_frame = ReplayStorageCapacity();
    _snapshot.replay_storage_capacity_after_last_frame = ReplayStorageCapacity();
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

    if (_bindingCount >= _bindings.size()) {
        return InputBindingResult{RecordFailure(InputStatus::CapacityExceeded), action};
    }

    _bindings[_bindingCount] = InputActionBinding{device, control, action};
    ++_bindingCount;
    _snapshot.binding_count = _bindingCount;

    if (!_registeredActions[action.value]) {
        _registeredActions[action.value] = true;
        ++_snapshot.action_count;
    }

    return InputBindingResult{InputStatus::Success, action};
}

InputStatus InputReplay::RecordReplayEvent(std::size_t frameIndex, InputEvent event) {
    if (frameIndex >= _frames.size()) {
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

    InputReplayFrame& frame = _frames[frameIndex];
    if (frame.event_count >= frame.events.size()) {
        return RejectReplayEvent(InputStatus::CapacityExceeded);
    }

    frame.events[frame.event_count] = event;
    ++frame.event_count;
    ++_snapshot.accepted_event_count;

    const std::size_t recordedFrameCount = frameIndex + 1U;
    if (recordedFrameCount > _recordedFrameCount) {
        _recordedFrameCount = recordedFrameCount;
    }

    return InputStatus::Success;
}

InputApplyResult InputReplay::ApplyNextFrame() {
    if (_nextFrameIndex >= _recordedFrameCount) {
        _snapshot.last_apply_status = InputStatus::EndOfReplay;
        return InputApplyResult{RecordFailure(InputStatus::EndOfReplay), _nextFrameIndex};
    }

    ResetFrameState();
    _snapshot.replay_storage_capacity_before_frame = ReplayStorageCapacity();

    InputStatus frameStatus = InputStatus::Success;
    const InputReplayFrame& frame = _frames[_nextFrameIndex];
    for (std::size_t eventIndex = 0U; eventIndex < frame.event_count; ++eventIndex) {
        const InputEvent& event = frame.events[eventIndex];
        const InputActionBinding* binding = FindBinding(event.device, event.control);
        if (binding == nullptr) {
            RejectReplayEvent(InputStatus::UnknownDeviceControl);
            frameStatus = InputStatus::UnknownDeviceControl;
            continue;
        }

        InputActionState& action = _actions[binding->action.value];
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
        frameStatus = InputStatus::InvalidEvent;
    }

    RecalculateChangedActionCount();
    ++_snapshot.apply_count;
    _snapshot.last_apply_status = frameStatus;
    _snapshot.replay_storage_capacity_after_last_frame = ReplayStorageCapacity();

    const std::size_t appliedFrameIndex = _nextFrameIndex;
    ++_nextFrameIndex;
    return InputApplyResult{frameStatus, appliedFrameIndex};
}

InputStatus InputReplay::ResetFrameState() {
    for (InputActionState& action : _actions) {
        action.changed_this_frame = false;
    }

    _snapshot.changed_action_count = 0U;
    ++_snapshot.reset_count;
    return InputStatus::Success;
}

InputActionQueryResult InputReplay::QueryAction(InputActionId action) const {
    if (!IsActionInRange(action)) {
        return InputActionQueryResult{InputStatus::UnknownAction, InputActionState{}};
    }

    if (!_registeredActions[action.value]) {
        return InputActionQueryResult{InputStatus::UnknownAction, InputActionState{}};
    }

    return InputActionQueryResult{InputStatus::Success, _actions[action.value]};
}

InputReplaySnapshot InputReplay::Snapshot() const {
    return _snapshot;
}

std::size_t InputReplay::EventCountForFrame(std::size_t frameIndex) const {
    if (frameIndex >= _frames.size()) {
        return 0U;
    }

    return _frames[frameIndex].event_count;
}

InputStatus InputReplay::RecordFailure(InputStatus status) {
    ++_snapshot.failed_operation_count;
    return status;
}

InputStatus InputReplay::RejectReplayEvent(InputStatus status) {
    ++_snapshot.rejected_event_count;
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
    for (std::size_t index = 0U; index < _bindingCount; ++index) {
        const InputActionBinding& binding = _bindings[index];
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
    _actions[action.value].changed_this_frame = true;
}

void InputReplay::RecalculateChangedActionCount() {
    std::size_t changedCount = 0U;
    for (std::size_t index = 0U; index < _actions.size(); ++index) {
        if (!_registeredActions[index]) {
            continue;
        }

        if (!_actions[index].changed_this_frame) {
            continue;
        }

        ++changedCount;
    }

    _snapshot.changed_action_count = changedCount;
}

std::size_t InputReplay::ReplayStorageCapacity() const {
    return MAX_REPLAY_FRAMES * MAX_EVENTS_PER_FRAME;
}
}
