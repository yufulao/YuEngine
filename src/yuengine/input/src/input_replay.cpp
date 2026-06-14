#include "yuengine/input/input_replay.h"

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
    _snapshot.DeviceCapacity = MAX_INPUT_DEVICES;
    _snapshot.ActionCapacity = MAX_INPUT_ACTIONS;
    _snapshot.BindingCapacity = MAX_INPUT_BINDINGS;
    _snapshot.ReplayFrameCapacity = MAX_REPLAY_FRAMES;
    _snapshot.EventCapacityPerFrame = MAX_EVENTS_PER_FRAME;
    _snapshot.ReplayStorageCapacityBeforeFrame = ReplayStorageCapacity();
    _snapshot.ReplayStorageCapacityAfterLastFrame = ReplayStorageCapacity();
}

InputBindingResult InputReplay::RegisterActionBinding(InputDeviceId device, InputControlId control, InputActionId action) {
    if (!IsDeviceValid(device)) {
        return InputBindingResult{RecordFailure(INPUT_STATUS::UnknownDeviceControl), action};
    }

    if (!IsActionInRange(action)) {
        return InputBindingResult{RecordFailure(INPUT_STATUS::UnknownAction), action};
    }

    if (HasBindingForControl(device, control)) {
        return InputBindingResult{RecordFailure(INPUT_STATUS::DuplicateBinding), action};
    }

    if (_bindingCount >= _bindings.size()) {
        return InputBindingResult{RecordFailure(INPUT_STATUS::CapacityExceeded), action};
    }

    _bindings[_bindingCount] = InputActionBinding{device, control, action};
    ++_bindingCount;
    _snapshot.BindingCount = _bindingCount;

    if (!_registeredActions[action.Value]) {
        _registeredActions[action.Value] = true;
        ++_snapshot.ActionCount;
    }

    return InputBindingResult{INPUT_STATUS::Success, action};
}

INPUT_STATUS InputReplay::RecordReplayEvent(std::size_t frameIndex, InputEvent event) {
    if (frameIndex >= _frames.size()) {
        return RejectReplayEvent(INPUT_STATUS::CapacityExceeded);
    }

    if (!IsEventTypeKnown(event.Type)) {
        return RejectReplayEvent(INPUT_STATUS::InvalidEvent);
    }

    if (event.Type == INPUT_EVENT_TYPE::Axis) {
        if (!IsAxisValueValid(event.AxisValue)) {
            return RejectReplayEvent(INPUT_STATUS::InvalidAxisValue);
        }
    }

    if (FindBinding(event.Device, event.Control) == nullptr) {
        return RejectReplayEvent(INPUT_STATUS::UnknownDeviceControl);
    }

    InputReplayFrame& frame = _frames[frameIndex];
    if (frame.EventCount >= frame.Events.size()) {
        return RejectReplayEvent(INPUT_STATUS::CapacityExceeded);
    }

    frame.Events[frame.EventCount] = event;
    ++frame.EventCount;
    ++_snapshot.AcceptedEventCount;

    const std::size_t recordedFrameCount = frameIndex + 1U;
    if (recordedFrameCount > _recordedFrameCount) {
        _recordedFrameCount = recordedFrameCount;
    }

    return INPUT_STATUS::Success;
}

InputApplyResult InputReplay::ApplyNextFrame() {
    if (_nextFrameIndex >= _recordedFrameCount) {
        _snapshot.LastApplyStatus = INPUT_STATUS::EndOfReplay;
        return InputApplyResult{RecordFailure(INPUT_STATUS::EndOfReplay), _nextFrameIndex};
    }

    ResetFrameState();
    _snapshot.ReplayStorageCapacityBeforeFrame = ReplayStorageCapacity();

    INPUT_STATUS frameStatus = INPUT_STATUS::Success;
    const InputReplayFrame& frame = _frames[_nextFrameIndex];
    for (std::size_t eventIndex = 0U; eventIndex < frame.EventCount; ++eventIndex) {
        const InputEvent& event = frame.Events[eventIndex];
        const InputActionBinding* binding = FindBinding(event.Device, event.Control);
        if (binding == nullptr) {
            RejectReplayEvent(INPUT_STATUS::UnknownDeviceControl);
            frameStatus = INPUT_STATUS::UnknownDeviceControl;
            continue;
        }

        InputActionState& action = _actions[binding->Action.Value];
        if (event.Type == INPUT_EVENT_TYPE::ButtonPressed) {
            action.IsPressed = true;
            MarkActionChanged(binding->Action);
            continue;
        }

        if (event.Type == INPUT_EVENT_TYPE::ButtonReleased) {
            action.IsPressed = false;
            MarkActionChanged(binding->Action);
            continue;
        }

        if (event.Type == INPUT_EVENT_TYPE::Axis) {
            action.AxisValue = event.AxisValue;
            MarkActionChanged(binding->Action);
            continue;
        }

        RejectReplayEvent(INPUT_STATUS::InvalidEvent);
        frameStatus = INPUT_STATUS::InvalidEvent;
    }

    RecalculateChangedActionCount();
    ++_snapshot.ApplyCount;
    _snapshot.LastApplyStatus = frameStatus;
    _snapshot.ReplayStorageCapacityAfterLastFrame = ReplayStorageCapacity();

    const std::size_t appliedFrameIndex = _nextFrameIndex;
    ++_nextFrameIndex;
    return InputApplyResult{frameStatus, appliedFrameIndex};
}

INPUT_STATUS InputReplay::ResetFrameState() {
    for (InputActionState& action : _actions) {
        action.ChangedThisFrame = false;
    }

    _snapshot.ChangedActionCount = 0U;
    ++_snapshot.ResetCount;
    return INPUT_STATUS::Success;
}

InputActionQueryResult InputReplay::QueryAction(InputActionId action) const {
    if (!IsActionInRange(action)) {
        return InputActionQueryResult{INPUT_STATUS::UnknownAction, InputActionState{}};
    }

    if (!_registeredActions[action.Value]) {
        return InputActionQueryResult{INPUT_STATUS::UnknownAction, InputActionState{}};
    }

    return InputActionQueryResult{INPUT_STATUS::Success, _actions[action.Value]};
}

InputReplaySnapshot InputReplay::Snapshot() const {
    return _snapshot;
}

std::size_t InputReplay::EventCountForFrame(std::size_t frameIndex) const {
    if (frameIndex >= _frames.size()) {
        return 0U;
    }

    return _frames[frameIndex].EventCount;
}

INPUT_STATUS InputReplay::RecordFailure(INPUT_STATUS status) {
    ++_snapshot.FailedOperationCount;
    return status;
}

INPUT_STATUS InputReplay::RejectReplayEvent(INPUT_STATUS status) {
    ++_snapshot.RejectedEventCount;
    return RecordFailure(status);
}

bool InputReplay::IsDeviceValid(InputDeviceId device) const {
    return device.Value < MAX_INPUT_DEVICES;
}

bool InputReplay::IsActionInRange(InputActionId action) const {
    return action.Value < MAX_INPUT_ACTIONS;
}

bool InputReplay::IsEventTypeKnown(INPUT_EVENT_TYPE type) const {
    if (type == INPUT_EVENT_TYPE::ButtonPressed) {
        return true;
    }

    if (type == INPUT_EVENT_TYPE::ButtonReleased) {
        return true;
    }

    return type == INPUT_EVENT_TYPE::Axis;
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
        if (binding.Device.Value != device.Value) {
            continue;
        }

        if (binding.Control.Value != control.Value) {
            continue;
        }

        return &binding;
    }

    return nullptr;
}

void InputReplay::MarkActionChanged(InputActionId action) {
    _actions[action.Value].ChangedThisFrame = true;
}

void InputReplay::RecalculateChangedActionCount() {
    std::size_t changedCount = 0U;
    for (std::size_t index = 0U; index < _actions.size(); ++index) {
        if (!_registeredActions[index]) {
            continue;
        }

        if (!_actions[index].ChangedThisFrame) {
            continue;
        }

        ++changedCount;
    }

    _snapshot.ChangedActionCount = changedCount;
}

std::size_t InputReplay::ReplayStorageCapacity() const {
    return MAX_REPLAY_FRAMES * MAX_EVENTS_PER_FRAME;
}
}
