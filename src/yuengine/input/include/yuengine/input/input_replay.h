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

    input_binding_result_t RegisterActionBinding(input_device_id_t device, input_control_id_t control, input_action_id_t action);
    INPUT_STATUS RecordReplayEvent(std::size_t frameIndex, input_event_t event);
    input_apply_result_t ApplyNextFrame();
    INPUT_STATUS ResetFrameState();
    input_action_query_result_t QueryAction(input_action_id_t action) const;
    input_replay_snapshot_t Snapshot() const;
    std::size_t EventCountForFrame(std::size_t frameIndex) const;

private:
    INPUT_STATUS RecordFailure(INPUT_STATUS status);
    INPUT_STATUS RejectReplayEvent(INPUT_STATUS status);
    bool IsDeviceValid(input_device_id_t device) const;
    bool IsActionInRange(input_action_id_t action) const;
    bool IsEventTypeKnown(INPUT_EVENT_TYPE type) const;
    bool IsAxisValueValid(std::int32_t value) const;
    bool HasBindingForControl(input_device_id_t device, input_control_id_t control) const;
    const input_action_binding_t* FindBinding(input_device_id_t device, input_control_id_t control) const;
    void MarkActionChanged(input_action_id_t action);
    void RecalculateChangedActionCount();
    std::size_t ReplayStorageCapacity() const;

    std::array<input_action_binding_t, MAX_INPUT_BINDINGS> _bindings;
    std::array<input_replay_frame_t, MAX_REPLAY_FRAMES> _frames;
    std::array<input_action_state_t, MAX_INPUT_ACTIONS> _actions;
    std::array<bool, MAX_INPUT_ACTIONS> _registeredActions;
    input_replay_snapshot_t _snapshot;
    std::size_t _bindingCount;
    std::size_t _recordedFrameCount;
    std::size_t _nextFrameIndex;
};
}
