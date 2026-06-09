#pragma once

#include <array>
#include <cstddef>

#include "yuengine/input/InputActionBinding.h"
#include "yuengine/input/InputActionQueryResult.h"
#include "yuengine/input/InputActionState.h"
#include "yuengine/input/InputApplyResult.h"
#include "yuengine/input/InputBindingResult.h"
#include "yuengine/input/InputConstants.h"
#include "yuengine/input/InputEvent.h"
#include "yuengine/input/InputReplaySnapshot.h"
#include "yuengine/input/InputStatus.h"

namespace yuengine::input
{
class InputReplay final
{
public:
    InputReplay();

    InputBindingResult RegisterActionBinding(InputDeviceId device, InputControlId control, InputActionId action);
    InputStatus RecordReplayEvent(std::size_t frameIndex, InputEvent event);
    InputApplyResult ApplyNextFrame();
    InputStatus ResetFrameState();
    InputActionQueryResult QueryAction(InputActionId action) const;
    InputReplaySnapshot Snapshot() const;
    std::size_t EventCountForFrame(std::size_t frameIndex) const;

private:
    struct InputReplayFrame final
    {
        std::array<InputEvent, MAX_EVENTS_PER_FRAME> Events{};
        std::size_t EventCount = 0U;
    };

    InputStatus RecordFailure(InputStatus status);
    InputStatus RejectReplayEvent(InputStatus status);
    bool IsDeviceValid(InputDeviceId device) const;
    bool IsActionInRange(InputActionId action) const;
    bool IsEventTypeKnown(InputEventType type) const;
    bool IsAxisValueValid(std::int32_t value) const;
    bool HasBindingForControl(InputDeviceId device, InputControlId control) const;
    const InputActionBinding* FindBinding(InputDeviceId device, InputControlId control) const;
    void MarkActionChanged(InputActionId action);
    void RecalculateChangedActionCount();
    std::size_t ReplayStorageCapacity() const;

    std::array<InputActionBinding, MAX_INPUT_BINDINGS> _bindings;
    std::array<InputReplayFrame, MAX_REPLAY_FRAMES> _frames;
    std::array<InputActionState, MAX_INPUT_ACTIONS> _actions;
    std::array<bool, MAX_INPUT_ACTIONS> _registeredActions;
    InputReplaySnapshot _snapshot;
    std::size_t _bindingCount;
    std::size_t _recordedFrameCount;
    std::size_t _nextFrameIndex;
};
}
