#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/Input/InputConstants.h"
#include "YuEngine/Input/InputReplay.h"

using yuengine::input::InputActionId;
using yuengine::input::InputActionState;
using yuengine::input::InputControlId;
using yuengine::input::InputDeviceId;
using yuengine::input::InputEvent;
using yuengine::input::InputEventType;
using InputReplay = yuengine::input::InputReplay;
using yuengine::input::InputReplaySnapshot;
using yuengine::input::InputStatus;
using yuengine::input::AXIS_MAX_VALUE;
using yuengine::input::AXIS_MIN_VALUE;
using yuengine::input::MAX_EVENTS_PER_FRAME;
using yuengine::input::MAX_INPUT_BINDINGS;
using yuengine::input::MAX_REPLAY_FRAMES;

namespace {
constexpr const char* TEST_REGISTER_BINDING = "Input_RegisterActionBinding_ReturnsStableActionId";
constexpr const char* TEST_DUPLICATE_CONTROL = "Input_RegisterControlAlreadyBound_ReturnsDuplicateStatus";
constexpr const char* TEST_MULTI_CONTROL_ORDER = "Input_MultipleControlsForOneAction_UsesInsertionOrder";
constexpr const char* TEST_BINDING_CAPACITY = "Input_BindingCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_PRESS_RELEASE = "Input_ReplayFrame_AppliesButtonPressAndRelease";
constexpr const char* TEST_EVENT_ORDER = "Input_ReplayFrame_EventOrderIsDeterministic";
constexpr const char* TEST_LAST_VALID_WINS = "Input_ReplayFrame_EventOrderLastValidValueWins";
constexpr const char* TEST_PRESS_RELEASE_CHANGED = "Input_ReplayFrame_PressReleaseSameFrame_SetsChangedFlag";
constexpr const char* TEST_AXIS = "Input_ReplayFrame_AppliesFixedPointAxisValue";
constexpr const char* TEST_INVALID_AXIS = "Input_InvalidAxisValue_ReturnsExplicitStatusWithoutMutation";
constexpr const char* TEST_INVALID_EVENT = "Input_InvalidEvent_DoesNotMutateReplayOrSnapshot";
constexpr const char* TEST_UNKNOWN_IDS = "Input_UnknownDeviceControlOrAction_ReturnsExplicitStatus";
constexpr const char* TEST_EVENT_CAPACITY = "Input_EventCapacityOverflow_DoesNotMutateReplay";
constexpr const char* TEST_SNAPSHOT_DETERMINISM = "Input_FrameSnapshot_IsDeterministicAcrossReplay";
constexpr const char* TEST_RESET = "Input_ResetClearsChangedStateWithoutClearingPressedState";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "Input_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_GROW = "Input_FrameApply_DoesNotGrowReplayStorage";
constexpr const char* TEST_NO_FORBIDDEN_DEPENDENCY = "Input_NoPlatformUiOrGameAdapterDependency";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";

constexpr InputDeviceId DEVICE_A{0U};
constexpr InputDeviceId DEVICE_B{1U};
constexpr InputDeviceId UNKNOWN_DEVICE{99U};
constexpr InputControlId CONTROL_A{10U};
constexpr InputControlId CONTROL_B{11U};
constexpr InputControlId UNKNOWN_CONTROL{99U};
constexpr InputActionId ACTION_A{0U};
constexpr InputActionId ACTION_B{1U};
constexpr InputActionId UNKNOWN_ACTION{99U};
constexpr std::int32_t AXIS_POSITIVE = 12345;
constexpr std::int32_t AXIS_NEGATIVE = -12345;
using TestFunction = int (*)();

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

InputEvent ButtonPress(InputDeviceId device, InputControlId control) {
    return InputEvent{device, control, InputEventType::ButtonPressed, 0};
}

InputEvent ButtonRelease(InputDeviceId device, InputControlId control) {
    return InputEvent{device, control, InputEventType::ButtonReleased, 0};
}

InputEvent Axis(InputDeviceId device, InputControlId control, std::int32_t value) {
    return InputEvent{device, control, InputEventType::Axis, value};
}

bool RegisterPrimaryBinding(InputReplay& replay) {
    return replay.RegisterActionBinding(DEVICE_A, CONTROL_A, ACTION_A).status == InputStatus::Success;
}

bool RegisterSecondaryBinding(InputReplay& replay) {
    return replay.RegisterActionBinding(DEVICE_B, CONTROL_B, ACTION_A).status == InputStatus::Success;
}

bool RegisterSecondActionBinding(InputReplay& replay) {
    return replay.RegisterActionBinding(DEVICE_A, CONTROL_B, ACTION_B).status == InputStatus::Success;
}

bool StateEquals(const InputActionState& left, const InputActionState& right) {
    if (left.is_pressed != right.is_pressed) {
        return false;
    }

    if (left.changed_this_frame != right.changed_this_frame) {
        return false;
    }

    return left.axis_value == right.axis_value;
}

bool SnapshotCountersEqual(const InputReplaySnapshot& left, const InputReplaySnapshot& right) {
    if (left.accepted_event_count != right.accepted_event_count) {
        return false;
    }

    if (left.rejected_event_count != right.rejected_event_count) {
        return false;
    }

    if (left.apply_count != right.apply_count) {
        return false;
    }

    return left.changed_action_count == right.changed_action_count;
}

int InputRegisterActionBindingReturnsStableActionId() {
    InputReplay replay;
    const auto result = replay.RegisterActionBinding(DEVICE_A, CONTROL_A, ACTION_A);
    if (result.status != InputStatus::Success) {
        return Fail("binding registration failed");
    }

    if (result.action.value != ACTION_A.value) {
        return Fail("binding did not return stable action id");
    }

    const auto snapshot = replay.Snapshot();
    if (snapshot.action_count != 1U) {
        return Fail("action count did not update");
    }

    if (snapshot.binding_count != 1U) {
        return Fail("binding count did not update");
    }

    return 0;
}

int InputRegisterControlAlreadyBoundReturnsDuplicateStatus() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("initial binding failed");
    }

    const auto before_snapshot = replay.Snapshot();
    const auto duplicate = replay.RegisterActionBinding(DEVICE_A, CONTROL_A, ACTION_B);
    if (duplicate.status != InputStatus::DuplicateBinding) {
        return Fail("duplicate control did not return duplicate status");
    }

    const auto after_snapshot = replay.Snapshot();
    if (after_snapshot.binding_count != before_snapshot.binding_count) {
        return Fail("duplicate control mutated binding count");
    }

    if (after_snapshot.action_count != before_snapshot.action_count) {
        return Fail("duplicate control mutated action count");
    }

    return 0;
}

int InputMultipleControlsForOneActionUsesInsertionOrder() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("primary binding failed");
    }

    if (!RegisterSecondaryBinding(replay)) {
        return Fail("secondary binding failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.RecordReplayEvent(0U, ButtonRelease(DEVICE_B, CONTROL_B));
    if (replay.ApplyNextFrame().status != InputStatus::Success) {
        return Fail("frame apply failed");
    }

    const auto state = replay.QueryAction(ACTION_A);
    if (state.status != InputStatus::Success) {
        return Fail("action query failed");
    }

    if (state.state.is_pressed) {
        return Fail("last multi-control event did not win");
    }

    if (!state.state.changed_this_frame) {
        return Fail("multi-control events did not set changed flag");
    }

    return 0;
}

int InputBindingCapacityOverflowDoesNotMutate() {
    InputReplay replay;
    for (std::size_t index = 0U; index < MAX_INPUT_BINDINGS; ++index) {
        const auto result = replay.RegisterActionBinding(DEVICE_A, InputControlId{static_cast<std::uint32_t>(index)}, ACTION_A);
        if (result.status != InputStatus::Success) {
            return Fail("binding failed before capacity");
        }
    }

    const auto before_snapshot = replay.Snapshot();
    const auto overflow = replay.RegisterActionBinding(DEVICE_A, InputControlId{999U}, ACTION_B);
    if (overflow.status != InputStatus::CapacityExceeded) {
        return Fail("binding overflow did not return capacity status");
    }

    if (replay.Snapshot().binding_count != before_snapshot.binding_count) {
        return Fail("binding overflow mutated binding count");
    }

    return 0;
}

int InputReplayFrameAppliesButtonPressAndRelease() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.RecordReplayEvent(1U, ButtonRelease(DEVICE_A, CONTROL_A));
    replay.ApplyNextFrame();
    if (!replay.QueryAction(ACTION_A).state.is_pressed) {
        return Fail("button press did not set pressed state");
    }

    replay.ApplyNextFrame();
    if (replay.QueryAction(ACTION_A).state.is_pressed) {
        return Fail("button release did not clear pressed state");
    }

    if (replay.ApplyNextFrame().status != InputStatus::EndOfReplay) {
        return Fail("replay end did not return explicit status");
    }

    return 0;
}

int InputReplayFrameEventOrderIsDeterministic() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.RecordReplayEvent(0U, ButtonRelease(DEVICE_A, CONTROL_A));
    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.ApplyNextFrame();

    const auto state = replay.QueryAction(ACTION_A).state;
    if (!state.is_pressed) {
        return Fail("accepted insertion order was not deterministic");
    }

    if (!state.changed_this_frame) {
        return Fail("ordered events did not set changed flag");
    }

    return 0;
}

int InputReplayFrameEventOrderLastValidValueWins() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, Axis(DEVICE_A, CONTROL_A, AXIS_NEGATIVE));
    replay.RecordReplayEvent(0U, Axis(DEVICE_A, CONTROL_A, AXIS_POSITIVE));
    replay.ApplyNextFrame();

    if (replay.QueryAction(ACTION_A).state.axis_value != AXIS_POSITIVE) {
        return Fail("last axis value did not win");
    }

    return 0;
}

int InputReplayFramePressReleaseSameFrameSetsChangedFlag() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.RecordReplayEvent(0U, ButtonRelease(DEVICE_A, CONTROL_A));
    replay.ApplyNextFrame();

    const auto state = replay.QueryAction(ACTION_A).state;
    if (state.is_pressed) {
        return Fail("press-release same frame changed final pressed state");
    }

    if (!state.changed_this_frame) {
        return Fail("press-release same frame did not set changed flag");
    }

    return 0;
}

int InputReplayFrameAppliesFixedPointAxisValue() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, Axis(DEVICE_A, CONTROL_A, AXIS_MIN_VALUE));
    replay.ApplyNextFrame();
    if (replay.QueryAction(ACTION_A).state.axis_value != AXIS_MIN_VALUE) {
        return Fail("minimum axis value did not apply");
    }

    replay.RecordReplayEvent(1U, Axis(DEVICE_A, CONTROL_A, AXIS_MAX_VALUE));
    replay.ApplyNextFrame();
    if (replay.QueryAction(ACTION_A).state.axis_value != AXIS_MAX_VALUE) {
        return Fail("maximum axis value did not apply");
    }

    return 0;
}

int InputInvalidAxisValueReturnsExplicitStatusWithoutMutation() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    const auto before_snapshot = replay.Snapshot();
    const InputStatus status = replay.RecordReplayEvent(0U, Axis(DEVICE_A, CONTROL_A, AXIS_MAX_VALUE + 1));
    if (status != InputStatus::InvalidAxisValue) {
        return Fail("invalid axis did not return invalid value status");
    }

    if (replay.EventCountForFrame(0U) != 0U) {
        return Fail("invalid axis mutated replay frame");
    }

    if (replay.Snapshot().accepted_event_count != before_snapshot.accepted_event_count) {
        return Fail("invalid axis mutated accepted event count");
    }

    if (replay.QueryAction(ACTION_A).state.axis_value != 0) {
        return Fail("invalid axis mutated snapshot value");
    }

    return 0;
}

int InputInvalidEventDoesNotMutateReplayOrSnapshot() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    const auto before_snapshot = replay.Snapshot();
    const InputEvent invalid_event{DEVICE_A, CONTROL_A, static_cast<InputEventType>(99), 0};
    if (replay.RecordReplayEvent(0U, invalid_event) != InputStatus::InvalidEvent) {
        return Fail("invalid event did not return explicit status");
    }

    if (replay.EventCountForFrame(0U) != 0U) {
        return Fail("invalid event mutated replay storage");
    }

    if (replay.Snapshot().accepted_event_count != before_snapshot.accepted_event_count) {
        return Fail("invalid event mutated accepted count");
    }

    if (replay.QueryAction(ACTION_A).state.changed_this_frame) {
        return Fail("invalid event mutated snapshot state");
    }

    return 0;
}

int InputUnknownDeviceControlOrActionReturnsExplicitStatus() {
    InputReplay replay;
    if (replay.RegisterActionBinding(UNKNOWN_DEVICE, CONTROL_A, ACTION_A).status != InputStatus::UnknownDeviceControl) {
        return Fail("unknown device did not return explicit status");
    }

    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    if (replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, UNKNOWN_CONTROL)) != InputStatus::UnknownDeviceControl) {
        return Fail("unknown control did not return explicit status");
    }

    if (replay.QueryAction(UNKNOWN_ACTION).status != InputStatus::UnknownAction) {
        return Fail("unknown action did not return explicit status");
    }

    return 0;
}

int InputEventCapacityOverflowDoesNotMutateReplay() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    for (std::size_t index = 0U; index < MAX_EVENTS_PER_FRAME; ++index) {
        if (replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A)) != InputStatus::Success) {
            return Fail("event insert failed before capacity");
        }
    }

    const auto before_snapshot = replay.Snapshot();
    if (replay.RecordReplayEvent(0U, ButtonRelease(DEVICE_A, CONTROL_A)) != InputStatus::CapacityExceeded) {
        return Fail("event overflow did not return capacity status");
    }

    if (replay.EventCountForFrame(0U) != MAX_EVENTS_PER_FRAME) {
        return Fail("event overflow mutated frame event count");
    }

    if (replay.Snapshot().accepted_event_count != before_snapshot.accepted_event_count) {
        return Fail("event overflow mutated accepted event count");
    }

    return 0;
}

int InputFrameSnapshotIsDeterministicAcrossReplay() {
    InputReplay first_replay;
    InputReplay second_replay;
    if (!RegisterPrimaryBinding(first_replay)) {
        return Fail("first binding failed");
    }

    if (!RegisterPrimaryBinding(second_replay)) {
        return Fail("second binding failed");
    }

    first_replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    first_replay.RecordReplayEvent(1U, Axis(DEVICE_A, CONTROL_A, AXIS_POSITIVE));
    second_replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    second_replay.RecordReplayEvent(1U, Axis(DEVICE_A, CONTROL_A, AXIS_POSITIVE));
    first_replay.ApplyNextFrame();
    first_replay.ApplyNextFrame();
    second_replay.ApplyNextFrame();
    second_replay.ApplyNextFrame();

    if (!StateEquals(first_replay.QueryAction(ACTION_A).state, second_replay.QueryAction(ACTION_A).state)) {
        return Fail("action state was not deterministic across replay");
    }

    if (!SnapshotCountersEqual(first_replay.Snapshot(), second_replay.Snapshot())) {
        return Fail("snapshot counters were not deterministic across replay");
    }

    return 0;
}

int InputResetClearsChangedStateWithoutClearingPressedState() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.ApplyNextFrame();
    if (!replay.QueryAction(ACTION_A).state.changed_this_frame) {
        return Fail("apply did not set changed state");
    }

    replay.ResetFrameState();
    const auto state = replay.QueryAction(ACTION_A).state;
    if (!state.is_pressed) {
        return Fail("reset cleared pressed state");
    }

    if (state.changed_this_frame) {
        return Fail("reset did not clear changed state");
    }

    return 0;
}

int InputDisabledDiagnosticsDoesNotChangeResults() {
    InputReplay enabled_like_replay;
    InputReplay disabled_like_replay;
    RegisterPrimaryBinding(enabled_like_replay);
    RegisterPrimaryBinding(disabled_like_replay);
    enabled_like_replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    disabled_like_replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));

    const auto enabled_result = enabled_like_replay.ApplyNextFrame();
    const auto disabled_result = disabled_like_replay.ApplyNextFrame();
    if (enabled_result.status != disabled_result.status) {
        return Fail("disabled diagnostics changed apply status");
    }

    if (!StateEquals(enabled_like_replay.QueryAction(ACTION_A).state, disabled_like_replay.QueryAction(ACTION_A).state)) {
        return Fail("disabled diagnostics changed action state");
    }

    if (!SnapshotCountersEqual(enabled_like_replay.Snapshot(), disabled_like_replay.Snapshot())) {
        return Fail("disabled diagnostics changed counters");
    }

    return 0;
}

int InputFrameApplyDoesNotGrowReplayStorage() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.ApplyNextFrame();

    const auto snapshot = replay.Snapshot();
    if (snapshot.replay_storage_capacity_before_frame != snapshot.replay_storage_capacity_after_last_frame) {
        return Fail("replay storage capacity changed during frame apply");
    }

    if (snapshot.replay_storage_capacity_before_frame != MAX_REPLAY_FRAMES * MAX_EVENTS_PER_FRAME) {
        return Fail("replay storage capacity was unexpected");
    }

    return 0;
}

int InputNoPlatformUiOrGameAdapterDependency() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("minimal input binding path failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    if (replay.ApplyNextFrame().status != InputStatus::Success) {
        return Fail("minimal synthetic replay path failed");
    }

    if (!replay.QueryAction(ACTION_A).state.is_pressed) {
        return Fail("minimal synthetic snapshot path failed");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_REGISTER_BINDING, InputRegisterActionBindingReturnsStableActionId},
        {TEST_DUPLICATE_CONTROL, InputRegisterControlAlreadyBoundReturnsDuplicateStatus},
        {TEST_MULTI_CONTROL_ORDER, InputMultipleControlsForOneActionUsesInsertionOrder},
        {TEST_BINDING_CAPACITY, InputBindingCapacityOverflowDoesNotMutate},
        {TEST_PRESS_RELEASE, InputReplayFrameAppliesButtonPressAndRelease},
        {TEST_EVENT_ORDER, InputReplayFrameEventOrderIsDeterministic},
        {TEST_LAST_VALID_WINS, InputReplayFrameEventOrderLastValidValueWins},
        {TEST_PRESS_RELEASE_CHANGED, InputReplayFramePressReleaseSameFrameSetsChangedFlag},
        {TEST_AXIS, InputReplayFrameAppliesFixedPointAxisValue},
        {TEST_INVALID_AXIS, InputInvalidAxisValueReturnsExplicitStatusWithoutMutation},
        {TEST_INVALID_EVENT, InputInvalidEventDoesNotMutateReplayOrSnapshot},
        {TEST_UNKNOWN_IDS, InputUnknownDeviceControlOrActionReturnsExplicitStatus},
        {TEST_EVENT_CAPACITY, InputEventCapacityOverflowDoesNotMutateReplay},
        {TEST_SNAPSHOT_DETERMINISM, InputFrameSnapshotIsDeterministicAcrossReplay},
        {TEST_RESET, InputResetClearsChangedStateWithoutClearingPressedState},
        {TEST_DISABLED_DIAGNOSTICS, InputDisabledDiagnosticsDoesNotChangeResults},
        {TEST_NO_GROW, InputFrameApplyDoesNotGrowReplayStorage},
        {TEST_NO_FORBIDDEN_DEPENDENCY, InputNoPlatformUiOrGameAdapterDependency}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
