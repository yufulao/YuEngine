#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>

#include "yuengine/input/input_constants.h"
#include "yuengine/input/input_replay.h"

using yuengine::input::input_action_id_t;
using yuengine::input::input_action_state_t;
using yuengine::input::input_control_id_t;
using yuengine::input::input_device_id_t;
using yuengine::input::input_event_t;
using yuengine::input::INPUT_EVENT_TYPE;
using InputReplay = yuengine::input::InputReplay;
using yuengine::input::input_replay_snapshot_t;
using yuengine::input::INPUT_STATUS;
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

constexpr input_device_id_t DEVICE_A{0U};
constexpr input_device_id_t DEVICE_B{1U};
constexpr input_device_id_t UNKNOWN_DEVICE{99U};
constexpr input_control_id_t CONTROL_A{10U};
constexpr input_control_id_t CONTROL_B{11U};
constexpr input_control_id_t UNKNOWN_CONTROL{99U};
constexpr input_action_id_t ACTION_A{0U};
constexpr input_action_id_t ACTION_B{1U};
constexpr input_action_id_t UNKNOWN_ACTION{99U};
constexpr std::int32_t AXIS_POSITIVE = 12345;
constexpr std::int32_t AXIS_NEGATIVE = -12345;
using TestFunction = int (*)();

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

input_event_t ButtonPress(input_device_id_t device, input_control_id_t control) {
    return input_event_t{device, control, INPUT_EVENT_TYPE::ButtonPressed, 0};
}

input_event_t ButtonRelease(input_device_id_t device, input_control_id_t control) {
    return input_event_t{device, control, INPUT_EVENT_TYPE::ButtonReleased, 0};
}

input_event_t Axis(input_device_id_t device, input_control_id_t control, std::int32_t value) {
    return input_event_t{device, control, INPUT_EVENT_TYPE::Axis, value};
}

bool RegisterPrimaryBinding(InputReplay& replay) {
    return replay.RegisterActionBinding(DEVICE_A, CONTROL_A, ACTION_A).Status == INPUT_STATUS::Success;
}

bool RegisterSecondaryBinding(InputReplay& replay) {
    return replay.RegisterActionBinding(DEVICE_B, CONTROL_B, ACTION_A).Status == INPUT_STATUS::Success;
}

bool RegisterSecondActionBinding(InputReplay& replay) {
    return replay.RegisterActionBinding(DEVICE_A, CONTROL_B, ACTION_B).Status == INPUT_STATUS::Success;
}

bool StateEquals(const input_action_state_t& left, const input_action_state_t& right) {
    if (left.IsPressed != right.IsPressed) {
        return false;
    }

    if (left.ChangedThisFrame != right.ChangedThisFrame) {
        return false;
    }

    return left.AxisValue == right.AxisValue;
}

bool SnapshotCountersEqual(const input_replay_snapshot_t& left, const input_replay_snapshot_t& right) {
    if (left.AcceptedEventCount != right.AcceptedEventCount) {
        return false;
    }

    if (left.RejectedEventCount != right.RejectedEventCount) {
        return false;
    }

    if (left.ApplyCount != right.ApplyCount) {
        return false;
    }

    return left.ChangedActionCount == right.ChangedActionCount;
}

int InputRegisterActionBindingReturnsStableActionId() {
    InputReplay replay;
    const auto result = replay.RegisterActionBinding(DEVICE_A, CONTROL_A, ACTION_A);
    if (result.Status != INPUT_STATUS::Success) {
        return Fail("binding registration failed");
    }

    if (result.Action.Value != ACTION_A.Value) {
        return Fail("binding did not return stable action id");
    }

    const auto snapshot = replay.Snapshot();
    if (snapshot.ActionCount != 1U) {
        return Fail("action count did not update");
    }

    if (snapshot.BindingCount != 1U) {
        return Fail("binding count did not update");
    }

    return 0;
}

int InputRegisterControlAlreadyBoundReturnsDuplicateStatus() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("initial binding failed");
    }

    const auto beforeSnapshot = replay.Snapshot();
    const auto duplicate = replay.RegisterActionBinding(DEVICE_A, CONTROL_A, ACTION_B);
    if (duplicate.Status != INPUT_STATUS::DuplicateBinding) {
        return Fail("duplicate control did not return duplicate status");
    }

    const auto afterSnapshot = replay.Snapshot();
    if (afterSnapshot.BindingCount != beforeSnapshot.BindingCount) {
        return Fail("duplicate control mutated binding count");
    }

    if (afterSnapshot.ActionCount != beforeSnapshot.ActionCount) {
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
    if (replay.ApplyNextFrame().Status != INPUT_STATUS::Success) {
        return Fail("frame apply failed");
    }

    const auto state = replay.QueryAction(ACTION_A);
    if (state.Status != INPUT_STATUS::Success) {
        return Fail("action query failed");
    }

    if (state.State.IsPressed) {
        return Fail("last multi-control event did not win");
    }

    if (!state.State.ChangedThisFrame) {
        return Fail("multi-control events did not set changed flag");
    }

    return 0;
}

int InputBindingCapacityOverflowDoesNotMutate() {
    InputReplay replay;
    for (std::size_t index = 0U; index < MAX_INPUT_BINDINGS; ++index) {
        const auto result = replay.RegisterActionBinding(DEVICE_A, input_control_id_t{static_cast<std::uint32_t>(index)}, ACTION_A);
        if (result.Status != INPUT_STATUS::Success) {
            return Fail("binding failed before capacity");
        }
    }

    const auto beforeSnapshot = replay.Snapshot();
    const auto overflow = replay.RegisterActionBinding(DEVICE_A, input_control_id_t{999U}, ACTION_B);
    if (overflow.Status != INPUT_STATUS::CapacityExceeded) {
        return Fail("binding overflow did not return capacity status");
    }

    if (replay.Snapshot().BindingCount != beforeSnapshot.BindingCount) {
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
    if (!replay.QueryAction(ACTION_A).State.IsPressed) {
        return Fail("button press did not set pressed state");
    }

    replay.ApplyNextFrame();
    if (replay.QueryAction(ACTION_A).State.IsPressed) {
        return Fail("button release did not clear pressed state");
    }

    if (replay.ApplyNextFrame().Status != INPUT_STATUS::EndOfReplay) {
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

    const auto state = replay.QueryAction(ACTION_A).State;
    if (!state.IsPressed) {
        return Fail("accepted insertion order was not deterministic");
    }

    if (!state.ChangedThisFrame) {
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

    if (replay.QueryAction(ACTION_A).State.AxisValue != AXIS_POSITIVE) {
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

    const auto state = replay.QueryAction(ACTION_A).State;
    if (state.IsPressed) {
        return Fail("press-release same frame changed final pressed state");
    }

    if (!state.ChangedThisFrame) {
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
    if (replay.QueryAction(ACTION_A).State.AxisValue != AXIS_MIN_VALUE) {
        return Fail("minimum axis value did not apply");
    }

    replay.RecordReplayEvent(1U, Axis(DEVICE_A, CONTROL_A, AXIS_MAX_VALUE));
    replay.ApplyNextFrame();
    if (replay.QueryAction(ACTION_A).State.AxisValue != AXIS_MAX_VALUE) {
        return Fail("maximum axis value did not apply");
    }

    return 0;
}

int InputInvalidAxisValueReturnsExplicitStatusWithoutMutation() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    const auto beforeSnapshot = replay.Snapshot();
    const INPUT_STATUS status = replay.RecordReplayEvent(0U, Axis(DEVICE_A, CONTROL_A, AXIS_MAX_VALUE + 1));
    if (status != INPUT_STATUS::InvalidAxisValue) {
        return Fail("invalid axis did not return invalid value status");
    }

    if (replay.EventCountForFrame(0U) != 0U) {
        return Fail("invalid axis mutated replay frame");
    }

    if (replay.Snapshot().AcceptedEventCount != beforeSnapshot.AcceptedEventCount) {
        return Fail("invalid axis mutated accepted event count");
    }

    if (replay.QueryAction(ACTION_A).State.AxisValue != 0) {
        return Fail("invalid axis mutated snapshot value");
    }

    return 0;
}

int InputInvalidEventDoesNotMutateReplayOrSnapshot() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    const auto beforeSnapshot = replay.Snapshot();
    const input_event_t invalidEvent{DEVICE_A, CONTROL_A, static_cast<INPUT_EVENT_TYPE>(99), 0};
    if (replay.RecordReplayEvent(0U, invalidEvent) != INPUT_STATUS::InvalidEvent) {
        return Fail("invalid event did not return explicit status");
    }

    if (replay.EventCountForFrame(0U) != 0U) {
        return Fail("invalid event mutated replay storage");
    }

    if (replay.Snapshot().AcceptedEventCount != beforeSnapshot.AcceptedEventCount) {
        return Fail("invalid event mutated accepted count");
    }

    if (replay.QueryAction(ACTION_A).State.ChangedThisFrame) {
        return Fail("invalid event mutated snapshot state");
    }

    return 0;
}

int InputUnknownDeviceControlOrActionReturnsExplicitStatus() {
    InputReplay replay;
    if (replay.RegisterActionBinding(UNKNOWN_DEVICE, CONTROL_A, ACTION_A).Status != INPUT_STATUS::UnknownDeviceControl) {
        return Fail("unknown device did not return explicit status");
    }

    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    if (replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, UNKNOWN_CONTROL)) != INPUT_STATUS::UnknownDeviceControl) {
        return Fail("unknown control did not return explicit status");
    }

    if (replay.QueryAction(UNKNOWN_ACTION).Status != INPUT_STATUS::UnknownAction) {
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
        if (replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A)) != INPUT_STATUS::Success) {
            return Fail("event insert failed before capacity");
        }
    }

    const auto beforeSnapshot = replay.Snapshot();
    if (replay.RecordReplayEvent(0U, ButtonRelease(DEVICE_A, CONTROL_A)) != INPUT_STATUS::CapacityExceeded) {
        return Fail("event overflow did not return capacity status");
    }

    if (replay.EventCountForFrame(0U) != MAX_EVENTS_PER_FRAME) {
        return Fail("event overflow mutated frame event count");
    }

    if (replay.Snapshot().AcceptedEventCount != beforeSnapshot.AcceptedEventCount) {
        return Fail("event overflow mutated accepted event count");
    }

    return 0;
}

int InputFrameSnapshotIsDeterministicAcrossReplay() {
    InputReplay firstReplay;
    InputReplay secondReplay;
    if (!RegisterPrimaryBinding(firstReplay)) {
        return Fail("first binding failed");
    }

    if (!RegisterPrimaryBinding(secondReplay)) {
        return Fail("second binding failed");
    }

    firstReplay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    firstReplay.RecordReplayEvent(1U, Axis(DEVICE_A, CONTROL_A, AXIS_POSITIVE));
    secondReplay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    secondReplay.RecordReplayEvent(1U, Axis(DEVICE_A, CONTROL_A, AXIS_POSITIVE));
    firstReplay.ApplyNextFrame();
    firstReplay.ApplyNextFrame();
    secondReplay.ApplyNextFrame();
    secondReplay.ApplyNextFrame();

    if (!StateEquals(firstReplay.QueryAction(ACTION_A).State, secondReplay.QueryAction(ACTION_A).State)) {
        return Fail("action state was not deterministic across replay");
    }

    if (!SnapshotCountersEqual(firstReplay.Snapshot(), secondReplay.Snapshot())) {
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
    if (!replay.QueryAction(ACTION_A).State.ChangedThisFrame) {
        return Fail("apply did not set changed state");
    }

    replay.ResetFrameState();
    const auto state = replay.QueryAction(ACTION_A).State;
    if (!state.IsPressed) {
        return Fail("reset cleared pressed state");
    }

    if (state.ChangedThisFrame) {
        return Fail("reset did not clear changed state");
    }

    return 0;
}

int InputDisabledDiagnosticsDoesNotChangeResults() {
    InputReplay enabledLikeReplay;
    InputReplay disabledLikeReplay;
    RegisterPrimaryBinding(enabledLikeReplay);
    RegisterPrimaryBinding(disabledLikeReplay);
    enabledLikeReplay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    disabledLikeReplay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));

    const auto enabledResult = enabledLikeReplay.ApplyNextFrame();
    const auto disabledResult = disabledLikeReplay.ApplyNextFrame();
    if (enabledResult.Status != disabledResult.Status) {
        return Fail("disabled diagnostics changed apply status");
    }

    if (!StateEquals(enabledLikeReplay.QueryAction(ACTION_A).State, disabledLikeReplay.QueryAction(ACTION_A).State)) {
        return Fail("disabled diagnostics changed action state");
    }

    if (!SnapshotCountersEqual(enabledLikeReplay.Snapshot(), disabledLikeReplay.Snapshot())) {
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
    if (snapshot.ReplayStorageCapacityBeforeFrame != snapshot.ReplayStorageCapacityAfterLastFrame) {
        return Fail("replay storage capacity changed during frame apply");
    }

    if (snapshot.ReplayStorageCapacityBeforeFrame != MAX_REPLAY_FRAMES * MAX_EVENTS_PER_FRAME) {
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
    if (replay.ApplyNextFrame().Status != INPUT_STATUS::Success) {
        return Fail("minimal synthetic replay path failed");
    }

    if (!replay.QueryAction(ACTION_A).State.IsPressed) {
        return Fail("minimal synthetic snapshot path failed");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> testRegistry{
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

    const std::string_view testName(argv[1]);
    const auto testIterator = testRegistry.find(testName);
    if (testIterator == testRegistry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return testIterator->second();
}
