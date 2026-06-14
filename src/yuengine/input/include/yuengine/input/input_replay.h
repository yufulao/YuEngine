#pragma once

#include <array>
#include <cstddef>

#include "yuengine/input/input_action_binding.h"
#include "yuengine/input/input_action_query_result.h"
#include "yuengine/input/input_action_state.h"
#include "yuengine/input/input_apply_result.h"
#include "yuengine/input/input_binding_result.h"
#include "yuengine/input/input_constants.h"
#include "yuengine/input/input_event.h"
#include "yuengine/input/input_replay_snapshot.h"
#include "yuengine/input/input_status.h"
#include "yuengine/input/input_replay_frame.h"

namespace yuengine::input {
class InputReplay final {
public:
    InputReplay();

    InputBindingResult RegisterActionBinding(InputDeviceId device, InputControlId control, InputActionId action);
    INPUT_STATUS RecordReplayEvent(std::size_t frameIndex, InputEvent event);
    InputApplyResult ApplyNextFrame();
    INPUT_STATUS ResetFrameState();
    InputActionQueryResult QueryAction(InputActionId action) const;
    InputReplaySnapshot Snapshot() const;
    std::size_t EventCountForFrame(std::size_t frameIndex) const;

private:
    INPUT_STATUS RecordFailure(INPUT_STATUS status);
    INPUT_STATUS RejectReplayEvent(INPUT_STATUS status);
    bool IsDeviceValid(InputDeviceId device) const;
    bool IsActionInRange(InputActionId action) const;
    bool IsEventTypeKnown(INPUT_EVENT_TYPE type) const;
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
