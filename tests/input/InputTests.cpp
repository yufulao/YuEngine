#include <array>
#include <cstdint>
#include <iostream>
#include <string>

#include "yuengine/input/InputConstants.h"
#include "yuengine/input/InputReplay.h"

using InputActionId = yuengine::input::InputActionId;
using InputActionState = yuengine::input::InputActionState;
using InputControlId = yuengine::input::InputControlId;
using InputDeviceId = yuengine::input::InputDeviceId;
using InputEvent = yuengine::input::InputEvent;
using InputEventType = yuengine::input::InputEventType;
using InputReplay = yuengine::input::InputReplay;
using InputReplaySnapshot = yuengine::input::InputReplaySnapshot;
using InputStatus = yuengine::input::InputStatus;

namespace
{
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

int Fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

InputEvent ButtonPress(InputDeviceId device, InputControlId control)
{
    return InputEvent{device, control, InputEventType::ButtonPressed, 0};
}

InputEvent ButtonRelease(InputDeviceId device, InputControlId control)
{
    return InputEvent{device, control, InputEventType::ButtonReleased, 0};
}

InputEvent Axis(InputDeviceId device, InputControlId control, std::int32_t value)
{
    return InputEvent{device, control, InputEventType::Axis, value};
}

bool RegisterPrimaryBinding(InputReplay& replay)
{
    return replay.RegisterActionBinding(DEVICE_A, CONTROL_A, ACTION_A).Status == InputStatus::Success;
}

bool RegisterSecondaryBinding(InputReplay& replay)
{
    return replay.RegisterActionBinding(DEVICE_B, CONTROL_B, ACTION_A).Status == InputStatus::Success;
}

bool RegisterSecondActionBinding(InputReplay& replay)
{
    return replay.RegisterActionBinding(DEVICE_A, CONTROL_B, ACTION_B).Status == InputStatus::Success;
}

bool StateEquals(const InputActionState& left, const InputActionState& right)
{
    if (left.IsPressed != right.IsPressed)
    {
        return false;
    }

    if (left.ChangedThisFrame != right.ChangedThisFrame)
    {
        return false;
    }

    return left.AxisValue == right.AxisValue;
}

bool SnapshotCountersEqual(const InputReplaySnapshot& left, const InputReplaySnapshot& right)
{
    if (left.AcceptedEventCount != right.AcceptedEventCount)
    {
        return false;
    }

    if (left.RejectedEventCount != right.RejectedEventCount)
    {
        return false;
    }

    if (left.ApplyCount != right.ApplyCount)
    {
        return false;
    }

    return left.ChangedActionCount == right.ChangedActionCount;
}

int InputRegisterActionBindingReturnsStableActionId()
{
    InputReplay replay;
    const auto result = replay.RegisterActionBinding(DEVICE_A, CONTROL_A, ACTION_A);
    if (result.Status != InputStatus::Success)
    {
        return Fail("binding registration failed");
    }

    if (result.Action.Value != ACTION_A.Value)
    {
        return Fail("binding did not return stable action id");
    }

    const auto snapshot = replay.Snapshot();
    if (snapshot.ActionCount != 1U)
    {
        return Fail("action count did not update");
    }

    if (snapshot.BindingCount != 1U)
    {
        return Fail("binding count did not update");
    }

    return 0;
}

int InputRegisterControlAlreadyBoundReturnsDuplicateStatus()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("initial binding failed");
    }

    const auto beforeSnapshot = replay.Snapshot();
    const auto duplicate = replay.RegisterActionBinding(DEVICE_A, CONTROL_A, ACTION_B);
    if (duplicate.Status != InputStatus::DuplicateBinding)
    {
        return Fail("duplicate control did not return duplicate status");
    }

    const auto afterSnapshot = replay.Snapshot();
    if (afterSnapshot.BindingCount != beforeSnapshot.BindingCount)
    {
        return Fail("duplicate control mutated binding count");
    }

    if (afterSnapshot.ActionCount != beforeSnapshot.ActionCount)
    {
        return Fail("duplicate control mutated action count");
    }

    return 0;
}

int InputMultipleControlsForOneActionUsesInsertionOrder()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("primary binding failed");
    }

    if (!RegisterSecondaryBinding(replay))
    {
        return Fail("secondary binding failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.RecordReplayEvent(0U, ButtonRelease(DEVICE_B, CONTROL_B));
    if (replay.ApplyNextFrame().Status != InputStatus::Success)
    {
        return Fail("frame apply failed");
    }

    const auto state = replay.QueryAction(ACTION_A);
    if (state.Status != InputStatus::Success)
    {
        return Fail("action query failed");
    }

    if (state.State.IsPressed)
    {
        return Fail("last multi-control event did not win");
    }

    if (!state.State.ChangedThisFrame)
    {
        return Fail("multi-control events did not set changed flag");
    }

    return 0;
}

int InputBindingCapacityOverflowDoesNotMutate()
{
    InputReplay replay;
    for (std::size_t index = 0U; index < yuengine::input::MAX_INPUT_BINDINGS; ++index)
    {
        const auto result = replay.RegisterActionBinding(DEVICE_A, InputControlId{static_cast<std::uint32_t>(index)}, ACTION_A);
        if (result.Status != InputStatus::Success)
        {
            return Fail("binding failed before capacity");
        }
    }

    const auto beforeSnapshot = replay.Snapshot();
    const auto overflow = replay.RegisterActionBinding(DEVICE_A, InputControlId{999U}, ACTION_B);
    if (overflow.Status != InputStatus::CapacityExceeded)
    {
        return Fail("binding overflow did not return capacity status");
    }

    if (replay.Snapshot().BindingCount != beforeSnapshot.BindingCount)
    {
        return Fail("binding overflow mutated binding count");
    }

    return 0;
}

int InputReplayFrameAppliesButtonPressAndRelease()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.RecordReplayEvent(1U, ButtonRelease(DEVICE_A, CONTROL_A));
    replay.ApplyNextFrame();
    if (!replay.QueryAction(ACTION_A).State.IsPressed)
    {
        return Fail("button press did not set pressed state");
    }

    replay.ApplyNextFrame();
    if (replay.QueryAction(ACTION_A).State.IsPressed)
    {
        return Fail("button release did not clear pressed state");
    }

    if (replay.ApplyNextFrame().Status != InputStatus::EndOfReplay)
    {
        return Fail("replay end did not return explicit status");
    }

    return 0;
}

int InputReplayFrameEventOrderIsDeterministic()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.RecordReplayEvent(0U, ButtonRelease(DEVICE_A, CONTROL_A));
    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.ApplyNextFrame();

    const auto state = replay.QueryAction(ACTION_A).State;
    if (!state.IsPressed)
    {
        return Fail("accepted insertion order was not deterministic");
    }

    if (!state.ChangedThisFrame)
    {
        return Fail("ordered events did not set changed flag");
    }

    return 0;
}

int InputReplayFrameEventOrderLastValidValueWins()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, Axis(DEVICE_A, CONTROL_A, AXIS_NEGATIVE));
    replay.RecordReplayEvent(0U, Axis(DEVICE_A, CONTROL_A, AXIS_POSITIVE));
    replay.ApplyNextFrame();

    if (replay.QueryAction(ACTION_A).State.AxisValue != AXIS_POSITIVE)
    {
        return Fail("last axis value did not win");
    }

    return 0;
}

int InputReplayFramePressReleaseSameFrameSetsChangedFlag()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.RecordReplayEvent(0U, ButtonRelease(DEVICE_A, CONTROL_A));
    replay.ApplyNextFrame();

    const auto state = replay.QueryAction(ACTION_A).State;
    if (state.IsPressed)
    {
        return Fail("press-release same frame changed final pressed state");
    }

    if (!state.ChangedThisFrame)
    {
        return Fail("press-release same frame did not set changed flag");
    }

    return 0;
}

int InputReplayFrameAppliesFixedPointAxisValue()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, Axis(DEVICE_A, CONTROL_A, yuengine::input::AXIS_MIN_VALUE));
    replay.ApplyNextFrame();
    if (replay.QueryAction(ACTION_A).State.AxisValue != yuengine::input::AXIS_MIN_VALUE)
    {
        return Fail("minimum axis value did not apply");
    }

    replay.RecordReplayEvent(1U, Axis(DEVICE_A, CONTROL_A, yuengine::input::AXIS_MAX_VALUE));
    replay.ApplyNextFrame();
    if (replay.QueryAction(ACTION_A).State.AxisValue != yuengine::input::AXIS_MAX_VALUE)
    {
        return Fail("maximum axis value did not apply");
    }

    return 0;
}

int InputInvalidAxisValueReturnsExplicitStatusWithoutMutation()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("binding failed");
    }

    const auto beforeSnapshot = replay.Snapshot();
    const InputStatus status = replay.RecordReplayEvent(0U, Axis(DEVICE_A, CONTROL_A, yuengine::input::AXIS_MAX_VALUE + 1));
    if (status != InputStatus::InvalidAxisValue)
    {
        return Fail("invalid axis did not return invalid value status");
    }

    if (replay.EventCountForFrame(0U) != 0U)
    {
        return Fail("invalid axis mutated replay frame");
    }

    if (replay.Snapshot().AcceptedEventCount != beforeSnapshot.AcceptedEventCount)
    {
        return Fail("invalid axis mutated accepted event count");
    }

    if (replay.QueryAction(ACTION_A).State.AxisValue != 0)
    {
        return Fail("invalid axis mutated snapshot value");
    }

    return 0;
}

int InputInvalidEventDoesNotMutateReplayOrSnapshot()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("binding failed");
    }

    const auto beforeSnapshot = replay.Snapshot();
    const InputEvent invalidEvent{DEVICE_A, CONTROL_A, static_cast<InputEventType>(99), 0};
    if (replay.RecordReplayEvent(0U, invalidEvent) != InputStatus::InvalidEvent)
    {
        return Fail("invalid event did not return explicit status");
    }

    if (replay.EventCountForFrame(0U) != 0U)
    {
        return Fail("invalid event mutated replay storage");
    }

    if (replay.Snapshot().AcceptedEventCount != beforeSnapshot.AcceptedEventCount)
    {
        return Fail("invalid event mutated accepted count");
    }

    if (replay.QueryAction(ACTION_A).State.ChangedThisFrame)
    {
        return Fail("invalid event mutated snapshot state");
    }

    return 0;
}

int InputUnknownDeviceControlOrActionReturnsExplicitStatus()
{
    InputReplay replay;
    if (replay.RegisterActionBinding(UNKNOWN_DEVICE, CONTROL_A, ACTION_A).Status != InputStatus::UnknownDeviceControl)
    {
        return Fail("unknown device did not return explicit status");
    }

    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("binding failed");
    }

    if (replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, UNKNOWN_CONTROL)) != InputStatus::UnknownDeviceControl)
    {
        return Fail("unknown control did not return explicit status");
    }

    if (replay.QueryAction(UNKNOWN_ACTION).Status != InputStatus::UnknownAction)
    {
        return Fail("unknown action did not return explicit status");
    }

    return 0;
}

int InputEventCapacityOverflowDoesNotMutateReplay()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("binding failed");
    }

    for (std::size_t index = 0U; index < yuengine::input::MAX_EVENTS_PER_FRAME; ++index)
    {
        if (replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A)) != InputStatus::Success)
        {
            return Fail("event insert failed before capacity");
        }
    }

    const auto beforeSnapshot = replay.Snapshot();
    if (replay.RecordReplayEvent(0U, ButtonRelease(DEVICE_A, CONTROL_A)) != InputStatus::CapacityExceeded)
    {
        return Fail("event overflow did not return capacity status");
    }

    if (replay.EventCountForFrame(0U) != yuengine::input::MAX_EVENTS_PER_FRAME)
    {
        return Fail("event overflow mutated frame event count");
    }

    if (replay.Snapshot().AcceptedEventCount != beforeSnapshot.AcceptedEventCount)
    {
        return Fail("event overflow mutated accepted event count");
    }

    return 0;
}

int InputFrameSnapshotIsDeterministicAcrossReplay()
{
    InputReplay firstReplay;
    InputReplay secondReplay;
    if (!RegisterPrimaryBinding(firstReplay))
    {
        return Fail("first binding failed");
    }

    if (!RegisterPrimaryBinding(secondReplay))
    {
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

    if (!StateEquals(firstReplay.QueryAction(ACTION_A).State, secondReplay.QueryAction(ACTION_A).State))
    {
        return Fail("action state was not deterministic across replay");
    }

    if (!SnapshotCountersEqual(firstReplay.Snapshot(), secondReplay.Snapshot()))
    {
        return Fail("snapshot counters were not deterministic across replay");
    }

    return 0;
}

int InputResetClearsChangedStateWithoutClearingPressedState()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.ApplyNextFrame();
    if (!replay.QueryAction(ACTION_A).State.ChangedThisFrame)
    {
        return Fail("apply did not set changed state");
    }

    replay.ResetFrameState();
    const auto state = replay.QueryAction(ACTION_A).State;
    if (!state.IsPressed)
    {
        return Fail("reset cleared pressed state");
    }

    if (state.ChangedThisFrame)
    {
        return Fail("reset did not clear changed state");
    }

    return 0;
}

int InputDisabledDiagnosticsDoesNotChangeResults()
{
    InputReplay enabledLikeReplay;
    InputReplay disabledLikeReplay;
    RegisterPrimaryBinding(enabledLikeReplay);
    RegisterPrimaryBinding(disabledLikeReplay);
    enabledLikeReplay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    disabledLikeReplay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));

    const auto enabledResult = enabledLikeReplay.ApplyNextFrame();
    const auto disabledResult = disabledLikeReplay.ApplyNextFrame();
    if (enabledResult.Status != disabledResult.Status)
    {
        return Fail("disabled diagnostics changed apply status");
    }

    if (!StateEquals(enabledLikeReplay.QueryAction(ACTION_A).State, disabledLikeReplay.QueryAction(ACTION_A).State))
    {
        return Fail("disabled diagnostics changed action state");
    }

    if (!SnapshotCountersEqual(enabledLikeReplay.Snapshot(), disabledLikeReplay.Snapshot()))
    {
        return Fail("disabled diagnostics changed counters");
    }

    return 0;
}

int InputFrameApplyDoesNotGrowReplayStorage()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("binding failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    replay.ApplyNextFrame();

    const auto snapshot = replay.Snapshot();
    if (snapshot.ReplayStorageCapacityBeforeFrame != snapshot.ReplayStorageCapacityAfterLastFrame)
    {
        return Fail("replay storage capacity changed during frame apply");
    }

    if (snapshot.ReplayStorageCapacityBeforeFrame != yuengine::input::MAX_REPLAY_FRAMES * yuengine::input::MAX_EVENTS_PER_FRAME)
    {
        return Fail("replay storage capacity was unexpected");
    }

    return 0;
}

int InputNoPlatformUiOrGameAdapterDependency()
{
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay))
    {
        return Fail("minimal input binding path failed");
    }

    replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    if (replay.ApplyNextFrame().Status != InputStatus::Success)
    {
        return Fail("minimal synthetic replay path failed");
    }

    if (!replay.QueryAction(ACTION_A).State.IsPressed)
    {
        return Fail("minimal synthetic snapshot path failed");
    }

    return 0;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return Fail("expected one test name");
    }

    const std::string testName(argv[1]);
    if (testName == TEST_REGISTER_BINDING)
    {
        return InputRegisterActionBindingReturnsStableActionId();
    }

    if (testName == TEST_DUPLICATE_CONTROL)
    {
        return InputRegisterControlAlreadyBoundReturnsDuplicateStatus();
    }

    if (testName == TEST_MULTI_CONTROL_ORDER)
    {
        return InputMultipleControlsForOneActionUsesInsertionOrder();
    }

    if (testName == TEST_BINDING_CAPACITY)
    {
        return InputBindingCapacityOverflowDoesNotMutate();
    }

    if (testName == TEST_PRESS_RELEASE)
    {
        return InputReplayFrameAppliesButtonPressAndRelease();
    }

    if (testName == TEST_EVENT_ORDER)
    {
        return InputReplayFrameEventOrderIsDeterministic();
    }

    if (testName == TEST_LAST_VALID_WINS)
    {
        return InputReplayFrameEventOrderLastValidValueWins();
    }

    if (testName == TEST_PRESS_RELEASE_CHANGED)
    {
        return InputReplayFramePressReleaseSameFrameSetsChangedFlag();
    }

    if (testName == TEST_AXIS)
    {
        return InputReplayFrameAppliesFixedPointAxisValue();
    }

    if (testName == TEST_INVALID_AXIS)
    {
        return InputInvalidAxisValueReturnsExplicitStatusWithoutMutation();
    }

    if (testName == TEST_INVALID_EVENT)
    {
        return InputInvalidEventDoesNotMutateReplayOrSnapshot();
    }

    if (testName == TEST_UNKNOWN_IDS)
    {
        return InputUnknownDeviceControlOrActionReturnsExplicitStatus();
    }

    if (testName == TEST_EVENT_CAPACITY)
    {
        return InputEventCapacityOverflowDoesNotMutateReplay();
    }

    if (testName == TEST_SNAPSHOT_DETERMINISM)
    {
        return InputFrameSnapshotIsDeterministicAcrossReplay();
    }

    if (testName == TEST_RESET)
    {
        return InputResetClearsChangedStateWithoutClearingPressedState();
    }

    if (testName == TEST_DISABLED_DIAGNOSTICS)
    {
        return InputDisabledDiagnosticsDoesNotChangeResults();
    }

    if (testName == TEST_NO_GROW)
    {
        return InputFrameApplyDoesNotGrowReplayStorage();
    }

    if (testName == TEST_NO_FORBIDDEN_DEPENDENCY)
    {
        return InputNoPlatformUiOrGameAdapterDependency();
    }

    return Fail("unknown test name");
}
