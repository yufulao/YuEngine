#include "yuengine/input/InputReplay.h"

namespace yuengine::input
{
InputReplay::InputReplay()
    : _bindings{},
      _frames{},
      _actions{},
      _registeredActions{},
      _snapshot{},
      _bindingCount(0U),
      _recordedFrameCount(0U),
      _nextFrameIndex(0U)
{
    _snapshot.DeviceCapacity = MAX_INPUT_DEVICES;
    _snapshot.ActionCapacity = MAX_INPUT_ACTIONS;
    _snapshot.BindingCapacity = MAX_INPUT_BINDINGS;
    _snapshot.ReplayFrameCapacity = MAX_REPLAY_FRAMES;
    _snapshot.EventCapacityPerFrame = MAX_EVENTS_PER_FRAME;
    _snapshot.ReplayStorageCapacityBeforeFrame = ReplayStorageCapacity();
    _snapshot.ReplayStorageCapacityAfterLastFrame = ReplayStorageCapacity();
}

InputBindingResult InputReplay::RegisterActionBinding(InputDeviceId device, InputControlId control, InputActionId action)
{
    if (!IsDeviceValid(device))
    {
        return InputBindingResult{RecordFailure(InputStatus::UnknownDeviceControl), action};
    }

    if (!IsActionInRange(action))
    {
        return InputBindingResult{RecordFailure(InputStatus::UnknownAction), action};
    }

    if (HasBindingForControl(device, control))
    {
        return InputBindingResult{RecordFailure(InputStatus::DuplicateBinding), action};
    }

    if (_bindingCount >= _bindings.size())
    {
        return InputBindingResult{RecordFailure(InputStatus::CapacityExceeded), action};
    }

    _bindings[_bindingCount] = InputActionBinding{device, control, action};
    ++_bindingCount;
    _snapshot.BindingCount = _bindingCount;

    if (!_registeredActions[action.Value])
    {
        _registeredActions[action.Value] = true;
        ++_snapshot.ActionCount;
    }

    return InputBindingResult{InputStatus::Success, action};
}

InputStatus InputReplay::RecordReplayEvent(std::size_t frameIndex, InputEvent event)
{
    if (frameIndex >= _frames.size())
    {
        return RejectReplayEvent(InputStatus::CapacityExceeded);
    }

    if (!IsEventTypeKnown(event.Type))
    {
        return RejectReplayEvent(InputStatus::InvalidEvent);
    }

    if (event.Type == InputEventType::Axis)
    {
        if (!IsAxisValueValid(event.AxisValue))
        {
            return RejectReplayEvent(InputStatus::InvalidAxisValue);
        }
    }

    if (FindBinding(event.Device, event.Control) == nullptr)
    {
        return RejectReplayEvent(InputStatus::UnknownDeviceControl);
    }

    InputReplayFrame& frame = _frames[frameIndex];
    if (frame.EventCount >= frame.Events.size())
    {
        return RejectReplayEvent(InputStatus::CapacityExceeded);
    }

    frame.Events[frame.EventCount] = event;
    ++frame.EventCount;
    ++_snapshot.AcceptedEventCount;

    const std::size_t recordedFrameCount = frameIndex + 1U;
    if (recordedFrameCount > _recordedFrameCount)
    {
        _recordedFrameCount = recordedFrameCount;
    }

    return InputStatus::Success;
}

InputApplyResult InputReplay::ApplyNextFrame()
{
    if (_nextFrameIndex >= _recordedFrameCount)
    {
        _snapshot.LastApplyStatus = InputStatus::EndOfReplay;
        return InputApplyResult{RecordFailure(InputStatus::EndOfReplay), _nextFrameIndex};
    }

    ResetFrameState();
    _snapshot.ReplayStorageCapacityBeforeFrame = ReplayStorageCapacity();

    InputStatus frameStatus = InputStatus::Success;
    const InputReplayFrame& frame = _frames[_nextFrameIndex];
    for (std::size_t eventIndex = 0U; eventIndex < frame.EventCount; ++eventIndex)
    {
        const InputEvent& event = frame.Events[eventIndex];
        const InputActionBinding* binding = FindBinding(event.Device, event.Control);
        if (binding == nullptr)
        {
            RejectReplayEvent(InputStatus::UnknownDeviceControl);
            frameStatus = InputStatus::UnknownDeviceControl;
            continue;
        }

        InputActionState& action = _actions[binding->Action.Value];
        if (event.Type == InputEventType::ButtonPressed)
        {
            action.IsPressed = true;
            MarkActionChanged(binding->Action);
            continue;
        }

        if (event.Type == InputEventType::ButtonReleased)
        {
            action.IsPressed = false;
            MarkActionChanged(binding->Action);
            continue;
        }

        if (event.Type == InputEventType::Axis)
        {
            action.AxisValue = event.AxisValue;
            MarkActionChanged(binding->Action);
            continue;
        }

        RejectReplayEvent(InputStatus::InvalidEvent);
        frameStatus = InputStatus::InvalidEvent;
    }

    RecalculateChangedActionCount();
    ++_snapshot.ApplyCount;
    _snapshot.LastApplyStatus = frameStatus;
    _snapshot.ReplayStorageCapacityAfterLastFrame = ReplayStorageCapacity();

    const std::size_t appliedFrameIndex = _nextFrameIndex;
    ++_nextFrameIndex;
    return InputApplyResult{frameStatus, appliedFrameIndex};
}

InputStatus InputReplay::ResetFrameState()
{
    for (InputActionState& action : _actions)
    {
        action.ChangedThisFrame = false;
    }

    _snapshot.ChangedActionCount = 0U;
    ++_snapshot.ResetCount;
    return InputStatus::Success;
}

InputActionQueryResult InputReplay::QueryAction(InputActionId action) const
{
    if (!IsActionInRange(action))
    {
        return InputActionQueryResult{InputStatus::UnknownAction, InputActionState{}};
    }

    if (!_registeredActions[action.Value])
    {
        return InputActionQueryResult{InputStatus::UnknownAction, InputActionState{}};
    }

    return InputActionQueryResult{InputStatus::Success, _actions[action.Value]};
}

InputReplaySnapshot InputReplay::Snapshot() const
{
    return _snapshot;
}

std::size_t InputReplay::EventCountForFrame(std::size_t frameIndex) const
{
    if (frameIndex >= _frames.size())
    {
        return 0U;
    }

    return _frames[frameIndex].EventCount;
}

InputStatus InputReplay::RecordFailure(InputStatus status)
{
    ++_snapshot.FailedOperationCount;
    return status;
}

InputStatus InputReplay::RejectReplayEvent(InputStatus status)
{
    ++_snapshot.RejectedEventCount;
    return RecordFailure(status);
}

bool InputReplay::IsDeviceValid(InputDeviceId device) const
{
    return device.Value < MAX_INPUT_DEVICES;
}

bool InputReplay::IsActionInRange(InputActionId action) const
{
    return action.Value < MAX_INPUT_ACTIONS;
}

bool InputReplay::IsEventTypeKnown(InputEventType type) const
{
    if (type == InputEventType::ButtonPressed)
    {
        return true;
    }

    if (type == InputEventType::ButtonReleased)
    {
        return true;
    }

    return type == InputEventType::Axis;
}

bool InputReplay::IsAxisValueValid(std::int32_t value) const
{
    if (value < AXIS_MIN_VALUE)
    {
        return false;
    }

    return value <= AXIS_MAX_VALUE;
}

bool InputReplay::HasBindingForControl(InputDeviceId device, InputControlId control) const
{
    return FindBinding(device, control) != nullptr;
}

const InputActionBinding* InputReplay::FindBinding(InputDeviceId device, InputControlId control) const
{
    for (std::size_t index = 0U; index < _bindingCount; ++index)
    {
        const InputActionBinding& binding = _bindings[index];
        if (binding.Device.Value != device.Value)
        {
            continue;
        }

        if (binding.Control.Value != control.Value)
        {
            continue;
        }

        return &binding;
    }

    return nullptr;
}

void InputReplay::MarkActionChanged(InputActionId action)
{
    _actions[action.Value].ChangedThisFrame = true;
}

void InputReplay::RecalculateChangedActionCount()
{
    std::size_t changedCount = 0U;
    for (std::size_t index = 0U; index < _actions.size(); ++index)
    {
        if (!_registeredActions[index])
        {
            continue;
        }

        if (!_actions[index].ChangedThisFrame)
        {
            continue;
        }

        ++changedCount;
    }

    _snapshot.ChangedActionCount = changedCount;
}

std::size_t InputReplay::ReplayStorageCapacity() const
{
    return MAX_REPLAY_FRAMES * MAX_EVENTS_PER_FRAME;
}
}
