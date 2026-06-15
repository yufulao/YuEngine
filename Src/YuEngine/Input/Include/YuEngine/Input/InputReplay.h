// Module: YuEngine Input
// File: Src/YuEngine/Input/Include/YuEngine/Input/InputReplay.h

#pragma once

#include <array>
#include <cstddef>

#include "YuEngine/Input/InputActionBinding.h"
#include "YuEngine/Input/InputActionQueryResult.h"
#include "YuEngine/Input/InputActionState.h"
#include "YuEngine/Input/InputApplyResult.h"
#include "YuEngine/Input/InputBindingResult.h"
#include "YuEngine/Input/InputConstants.h"
#include "YuEngine/Input/InputEvent.h"
#include "YuEngine/Input/InputReplaySnapshot.h"
#include "YuEngine/Input/InputStatus.h"
#include "YuEngine/Input/InputReplayFrame.h"

namespace yuengine::input {
class InputReplay final {
public:
    /**
     * @comment Constructs a InputReplay instance.
     */
    InputReplay();

    /**
     * @comment Registers action binding.
     * @param device Input device.
     * @param control Input control.
     * @param action Input action.
     * @return Explicit operation result.
     */
    InputBindingResult RegisterActionBinding(InputDeviceId device, InputControlId control, InputActionId action);
    /**
     * @comment Records replay event.
     * @param frame_index Input frame index.
     * @param event Input event.
     * @return Explicit operation status.
     */
    InputStatus RecordReplayEvent(std::size_t frame_index, InputEvent event);
    /**
     * @comment Applies next frame.
     * @return Explicit operation result.
     */
    InputApplyResult ApplyNextFrame();
    /**
     * @comment Resets frame state.
     * @return Explicit operation status.
     */
    InputStatus ResetFrameState();
    /**
     * @comment Queries action.
     * @param action Input action.
     * @return Explicit operation result.
     */
    InputActionQueryResult QueryAction(InputActionId action) const;
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    InputReplaySnapshot Snapshot() const;
    /**
     * @comment Returns the event count for a frame.
     * @param frame_index Input frame index.
     * @return Event count for frame value.
     */
    std::size_t EventCountForFrame(std::size_t frame_index) const;

private:
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

    std::array<InputActionBinding, MAX_INPUT_BINDINGS> bindings_;
    std::array<InputReplayFrame, MAX_REPLAY_FRAMES> frames_;
    std::array<InputActionState, MAX_INPUT_ACTIONS> actions_;
    std::array<bool, MAX_INPUT_ACTIONS> registered_actions_;
    InputReplaySnapshot snapshot_;
    std::size_t binding_count_;
    std::size_t recorded_frame_count_;
    std::size_t next_frame_index_;
};
}
