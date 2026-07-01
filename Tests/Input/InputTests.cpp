// 模块：Tests Input
// 路径：Tests/Input/InputTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include "YuEngine/Input/InputBackendKind.h"
#include "YuEngine/Input/InputBridge.h"
#include "YuEngine/Input/InputBridgeDesc.h"
#include "YuEngine/Input/InputBridgeEvent.h"
#include "YuEngine/Input/InputBridgeSnapshot.h"
#include "YuEngine/Input/InputCommandBinding.h"
#include "YuEngine/Input/InputCommandMapper.h"
#include "YuEngine/Input/InputCommandSnapshot.h"
#include "YuEngine/Input/InputCommandValueKind.h"
#include "YuEngine/Input/InputConstants.h"
#include "YuEngine/Input/InputContextFocusMode.h"
#include "YuEngine/Input/InputContextId.h"
#include "YuEngine/Input/InputDeviceKind.h"
#include "YuEngine/Input/InputGamepadState.h"
#include "YuEngine/Input/InputReplay.h"
#include "InputBridgeWindowsInternal.h"

using yuengine::input::InputActionId;
using yuengine::input::InputActionState;
using yuengine::input::InputBackendKind;
using yuengine::input::InputBindingResult;
using yuengine::input::InputBridge;
using yuengine::input::InputBridgeDesc;
using yuengine::input::InputBridgeEvent;
using yuengine::input::InputBridgeEventType;
using yuengine::input::InputBridgeSnapshot;
using yuengine::input::InputCommandBinding;
using yuengine::input::InputCommandMapper;
using yuengine::input::InputCommandMapperSnapshot;
using yuengine::input::InputCommandSnapshot;
using yuengine::input::InputCommandValueKind;
using yuengine::input::InputControlId;
using yuengine::input::InputContextFocusMode;
using yuengine::input::InputContextId;
using yuengine::input::InputDeviceId;
using yuengine::input::InputDeviceKind;
using yuengine::input::InputEvent;
using yuengine::input::InputEventType;
using yuengine::input::InputFocusPolicy;
using yuengine::input::InputGamepadConnection;
using yuengine::input::InputGamepadState;
using InputReplay = yuengine::input::InputReplay;
using yuengine::input::InputReplaySnapshot;
using yuengine::input::InputStatus;
using yuengine::input::AXIS_MAX_VALUE;
using yuengine::input::AXIS_MIN_VALUE;
using yuengine::input::GAMEPAD_BUTTON_A;
using yuengine::input::GAMEPAD_BUTTON_B;
using yuengine::input::GAMEPAD_LEFT_THUMB_X_CONTROL;
using yuengine::input::GAMEPAD_RIGHT_TRIGGER_CONTROL;
using yuengine::input::MAX_GAMEPAD_DEVICES;
using yuengine::input::MAX_EVENTS_PER_FRAME;
using yuengine::input::MAX_INPUT_BINDINGS;
using yuengine::input::MAX_INPUT_CONTEXTS;
using yuengine::input::MAX_REPLAY_FRAMES;
using yuengine::input::internal::InputNativeGamepadPollFunction;
using yuengine::input::internal::InputNativeGamepadPollStatus;
using yuengine::input::internal::InputNativeGamepadState;
using yuengine::input::internal::SetInputNativeGamepadPollFunctionForTest;

namespace {
constexpr const char* TEST_REGISTER_BINDING = "Input_RegisterActionBinding_ReturnsStableActionId";
constexpr const char* TEST_DUPLICATE_CONTROL = "Input_RegisterControlAlreadyBound_ReturnsDuplicateStatus";
constexpr const char* TEST_MULTI_CONTROL_ORDER = "Input_MultipleControlsForOneAction_UsesInsertionOrder";
constexpr const char* TEST_BINDING_CAPACITY = "Input_BindingCapacityOverflow_DoesNotMutate";
constexpr const char *TEST_BINDING_CAPACITY_ENTRY = "Input_BindingCapacityOverflow_ReportsRejectedEntry";
constexpr const char* TEST_PRESS_RELEASE = "Input_ReplayFrame_AppliesButtonPressAndRelease";
constexpr const char* TEST_EVENT_ORDER = "Input_ReplayFrame_EventOrderIsDeterministic";
constexpr const char* TEST_LAST_VALID_WINS = "Input_ReplayFrame_EventOrderLastValidValueWins";
constexpr const char* TEST_PRESS_RELEASE_CHANGED = "Input_ReplayFrame_PressReleaseSameFrame_SetsChangedFlag";
constexpr const char* TEST_AXIS = "Input_ReplayFrame_AppliesFixedPointAxisValue";
constexpr const char* TEST_INVALID_AXIS = "Input_InvalidAxisValue_ReturnsExplicitStatusWithoutMutation";
constexpr const char* TEST_INVALID_EVENT = "Input_InvalidEvent_DoesNotMutateReplayOrSnapshot";
constexpr const char* TEST_UNKNOWN_IDS = "Input_UnknownDeviceControlOrAction_ReturnsExplicitStatus";
constexpr const char* TEST_EVENT_CAPACITY = "Input_EventCapacityOverflow_DoesNotMutateReplay";
constexpr const char* TEST_REPLAY_CAPACITY_ENTRY = "Input_ReplayEventCapacityFailureReportsEntry";
constexpr const char* TEST_SNAPSHOT_DETERMINISM = "Input_FrameSnapshot_IsDeterministicAcrossReplay";
constexpr const char* TEST_RESET = "Input_ResetClearsChangedStateWithoutClearingPressedState";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "Input_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_GROW = "Input_FrameApply_DoesNotGrowReplayStorage";
constexpr const char* TEST_NO_FORBIDDEN_DEPENDENCY = "Input_NoPlatformUiOrGameAdapterDependency";
constexpr const char* TEST_BRIDGE_DESC = "Input_BridgeDesc_DefaultValuesAreBounded";
constexpr const char* TEST_BRIDGE_CONTRACT = "Input_BridgePublicContract_UsesValueTypes";
constexpr const char* TEST_BRIDGE_UNSUPPORTED = "Input_BridgeInitialize_RejectsUnsupportedBackend";
constexpr const char* TEST_BRIDGE_DRAIN = "Input_BridgeSubmitAndDrain_RecordsKeyboardMouseWheel";
constexpr const char* TEST_BRIDGE_FOCUS = "Input_BridgeFocusLost_RejectsInputAndTracksCounters";
constexpr const char* TEST_BRIDGE_CAPACITY = "Input_BridgeCapacityOverflow_DoesNotGrow";
constexpr const char* TEST_BRIDGE_SMALL_DRAIN = "Input_BridgeDrain_RejectsSmallOutputWithoutMutation";
constexpr const char* TEST_BRIDGE_GAMEPAD_UNAVAILABLE = "Input_BridgeGamepadUnavailableState_IsExplicit";
constexpr const char* TEST_BRIDGE_GAMEPAD_CONNECTED = "Input_BridgeGamepadConnectedState_QueuesButtonAndAxisEvents";
constexpr const char* TEST_BRIDGE_GAMEPAD_REPEAT = "Input_BridgeGamepadRepeatedPacket_DoesNotQueueDuplicateEvents";
constexpr const char* TEST_BRIDGE_GAMEPAD_CAPACITY = "Input_BridgeGamepadCapacityOverflow_DoesNotQueuePartialEvents";
constexpr const char *TEST_BRIDGE_XINPUT_GAMEPAD_CAPACITY = "Input_BridgeXInputPollGamepadCapacityOverflow_RecordsRejectedState";
constexpr const char *TEST_BRIDGE_XINPUT_UNAVAILABLE = "Input_BridgeXInputPollUnavailable_ReturnsDeviceUnavailable";
constexpr const char *TEST_BRIDGE_XINPUT_CONNECTED = "Input_BridgeXInputPollConnected_QueuesButtonAxisAndPacket";
constexpr const char *TEST_BRIDGE_XINPUT_BACKEND_ERROR = "Input_BridgeXInputPollBackendError_ReturnsExplicitStatus";
constexpr const char *TEST_BRIDGE_XINPUT_INVALID_USER = "Input_BridgeXInputPollInvalidUserIndex_ReturnsExplicitStatus";
constexpr const char* TEST_BRIDGE_NO_DISPATCH = "Input_BridgeNoUiGameOrReportDispatch";
constexpr const char *TEST_COMMAND_KEYBOARD = "Input_CommandMapper_KeyboardBuildsFrameCommandSnapshot";
constexpr const char *TEST_COMMAND_XINPUT_AXIS = "Input_CommandMapper_XInputStyleAxisBuildsCommandSnapshot";
constexpr const char *TEST_COMMAND_RUNTIME_BOUNDARY = "Input_CommandMapper_KeyboardAndXInputFixturesStayRuntimeOnly";
constexpr const char *TEST_COMMAND_FOCUS = "Input_CommandMapper_FocusRejectsInputWithoutMutation";
constexpr const char *TEST_COMMAND_INVALID = "Input_CommandMapper_InvalidEventDoesNotMutateHeldState";
constexpr const char *TEST_COMMAND_SETUP_REJECTION = "Input_CommandMapper_SetupFailuresDoNotCountRejectedEvents";
constexpr const char *TEST_COMMAND_CAPACITY_ENTRY = "Input_CommandMapper_CapacityFailuresRecordEntryIdentity";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";

constexpr InputContextId CONTEXT_A{0U};
constexpr InputContextId CONTEXT_B{1U};
constexpr InputContextId UNKNOWN_CONTEXT{static_cast<std::uint32_t>(MAX_INPUT_CONTEXTS)};
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

InputBridgeEvent BridgeKey(InputBridgeEventType type, std::uint32_t raw_code) {
    InputBridgeEvent event{};
    event.device_kind = InputDeviceKind::Keyboard;
    event.device = DEVICE_A;
    event.control = InputControlId{raw_code};
    event.type = type;
    event.raw_code = raw_code;
    return event;
}

InputBridgeEvent BridgeMouseMove(std::int32_t x, std::int32_t y) {
    InputBridgeEvent event{};
    event.device_kind = InputDeviceKind::Mouse;
    event.device = DEVICE_B;
    event.control = InputControlId{1U};
    event.type = InputBridgeEventType::MouseMoved;
    event.pointer_x = x;
    event.pointer_y = y;
    return event;
}

InputBridgeEvent BridgeMouseWheel(std::int32_t delta) {
    InputBridgeEvent event{};
    event.device_kind = InputDeviceKind::Mouse;
    event.device = DEVICE_B;
    event.control = InputControlId{2U};
    event.type = InputBridgeEventType::MouseWheel;
    event.wheel_delta = delta;
    event.axis_value = delta;
    return event;
}

InputGamepadState GamepadState(InputDeviceId device, std::uint32_t packet_number) {
    InputGamepadState state{};
    state.device = device;
    state.connection = InputGamepadConnection::Connected;
    state.packet_number = packet_number;
    return state;
}

InputGamepadState UnavailableGamepadState(InputDeviceId device) {
    InputGamepadState state{};
    state.device = device;
    state.connection = InputGamepadConnection::Unavailable;
    return state;
}

InputNativeGamepadPollStatus PollNativeUnavailable(std::uint32_t user_index, InputNativeGamepadState *state) {
    static_cast<void>(user_index);
    if (state == nullptr) {
        return InputNativeGamepadPollStatus::BackendError;
    }

    *state = InputNativeGamepadState{};
    return InputNativeGamepadPollStatus::DeviceUnavailable;
}

InputNativeGamepadPollStatus PollNativeConnected(std::uint32_t user_index, InputNativeGamepadState *state) {
    if (user_index != 0U) {
        return InputNativeGamepadPollStatus::BackendError;
    }

    if (state == nullptr) {
        return InputNativeGamepadPollStatus::BackendError;
    }

    state->packet_number = 31U;
    state->buttons = GAMEPAD_BUTTON_A;
    state->right_trigger = 64U;
    state->left_thumb_x = -1234;
    return InputNativeGamepadPollStatus::Success;
}

InputNativeGamepadPollStatus PollNativeCapacityState(std::uint32_t user_index, InputNativeGamepadState *state) {
    if (user_index != 0U) {
        return InputNativeGamepadPollStatus::BackendError;
    }

    if (state == nullptr) {
        return InputNativeGamepadPollStatus::BackendError;
    }

    state->packet_number = 44U;
    state->buttons = static_cast<std::uint16_t>(GAMEPAD_BUTTON_A | GAMEPAD_BUTTON_B);
    state->left_trigger = 9U;
    state->right_trigger = 10U;
    state->left_thumb_x = 111;
    state->left_thumb_y = -112;
    return InputNativeGamepadPollStatus::Success;
}

InputNativeGamepadPollStatus PollNativeBackendError(std::uint32_t user_index, InputNativeGamepadState *state) {
    static_cast<void>(user_index);
    if (state != nullptr) {
        *state = InputNativeGamepadState{};
    }

    return InputNativeGamepadPollStatus::BackendError;
}

class ScopedNativeGamepadPollFunction final {
public:
    explicit ScopedNativeGamepadPollFunction(InputNativeGamepadPollFunction function)
        : previous_function_(SetInputNativeGamepadPollFunctionForTest(function)) {
    }

    ~ScopedNativeGamepadPollFunction() {
        static_cast<void>(SetInputNativeGamepadPollFunctionForTest(previous_function_));
    }

    ScopedNativeGamepadPollFunction(const ScopedNativeGamepadPollFunction &) = delete;
    ScopedNativeGamepadPollFunction &operator=(const ScopedNativeGamepadPollFunction &) = delete;

private:
    InputNativeGamepadPollFunction previous_function_ = nullptr;
};

bool RegisterPrimaryBinding(InputReplay& replay) {
    return replay.RegisterActionBinding(DEVICE_A, CONTROL_A, ACTION_A).status == InputStatus::Success;
}

bool RegisterSecondaryBinding(InputReplay& replay) {
    return replay.RegisterActionBinding(DEVICE_B, CONTROL_B, ACTION_A).status == InputStatus::Success;
}

bool GamepadCapacityIdentityMatches(
    const InputBridgeSnapshot &snapshot,
    const InputGamepadState &state,
    std::size_t event_capacity,
    std::size_t event_count,
    std::size_t required_event_count) {
    if (snapshot.last_failed_gamepad_event_capacity != event_capacity) {
        return false;
    }

    if (snapshot.last_failed_gamepad_event_count != event_count) {
        return false;
    }

    if (snapshot.last_required_gamepad_event_count != required_event_count) {
        return false;
    }

    if (snapshot.last_failed_gamepad_device.value != state.device.value) {
        return false;
    }

    if (snapshot.last_failed_gamepad_connection != state.connection) {
        return false;
    }

    if (snapshot.last_failed_gamepad_packet_number != state.packet_number) {
        return false;
    }

    if (snapshot.last_failed_gamepad_button_bits != state.buttons) {
        return false;
    }

    if (snapshot.last_failed_gamepad_left_trigger != state.left_trigger) {
        return false;
    }

    if (snapshot.last_failed_gamepad_right_trigger != state.right_trigger) {
        return false;
    }

    if (snapshot.last_failed_gamepad_left_thumb_x != state.left_thumb_x) {
        return false;
    }

    if (snapshot.last_failed_gamepad_left_thumb_y != state.left_thumb_y) {
        return false;
    }

    if (snapshot.last_failed_gamepad_right_thumb_x != state.right_thumb_x) {
        return false;
    }

    return snapshot.last_failed_gamepad_right_thumb_y == state.right_thumb_y;
}

bool GamepadCapacityIdentityIsClear(const InputBridgeSnapshot &snapshot) {
    if (snapshot.last_failed_gamepad_event_capacity != 0U) {
        return false;
    }

    if (snapshot.last_failed_gamepad_event_count != 0U) {
        return false;
    }

    if (snapshot.last_required_gamepad_event_count != 0U) {
        return false;
    }

    if (snapshot.last_failed_gamepad_device.value != 0U) {
        return false;
    }

    if (snapshot.last_failed_gamepad_connection != InputGamepadConnection::Unavailable) {
        return false;
    }

    if (snapshot.last_failed_gamepad_packet_number != 0U) {
        return false;
    }

    if (snapshot.last_failed_gamepad_button_bits != 0U) {
        return false;
    }

    if (snapshot.last_failed_gamepad_left_trigger != 0U) {
        return false;
    }

    if (snapshot.last_failed_gamepad_right_trigger != 0U) {
        return false;
    }

    if (snapshot.last_failed_gamepad_left_thumb_x != 0) {
        return false;
    }

    if (snapshot.last_failed_gamepad_left_thumb_y != 0) {
        return false;
    }

    if (snapshot.last_failed_gamepad_right_thumb_x != 0) {
        return false;
    }

    return snapshot.last_failed_gamepad_right_thumb_y == 0;
}

bool RegisterSecondActionBinding(InputReplay& replay) {
    return replay.RegisterActionBinding(DEVICE_A, CONTROL_B, ACTION_B).status == InputStatus::Success;
}

InputCommandBinding ButtonCommandBinding(
    InputContextId context,
    InputDeviceId device,
    InputControlId control,
    InputActionId action) {
    InputCommandBinding binding{};
    binding.context = context;
    binding.device = device;
    binding.control = control;
    binding.action = action;
    binding.value_kind = InputCommandValueKind::Button;
    return binding;
}

InputCommandBinding AxisCommandBinding(
    InputContextId context,
    InputDeviceId device,
    InputControlId control,
    InputActionId action) {
    InputCommandBinding binding{};
    binding.context = context;
    binding.device = device;
    binding.control = control;
    binding.action = action;
    binding.value_kind = InputCommandValueKind::Axis;
    return binding;
}

bool RegisterCommandContext(InputCommandMapper &mapper) {
    if (mapper.RegisterContext(CONTEXT_A) != InputStatus::Success) {
        return false;
    }

    return mapper.SetActiveContext(CONTEXT_A, InputContextFocusMode::AcceptInput) == InputStatus::Success;
}

bool CommandMapperCapacityEntryIsClear(const InputCommandMapperSnapshot &snapshot) {
    if (snapshot.last_failed_capacity_context_id.value != 0U) {
        return false;
    }

    if (snapshot.last_failed_capacity_action_id.value != 0U) {
        return false;
    }

    if (snapshot.last_failed_capacity_device_id.value != 0U) {
        return false;
    }

    if (snapshot.last_failed_capacity_control_id.value != 0U) {
        return false;
    }

    if (snapshot.last_failed_binding_capacity != 0U) {
        return false;
    }

    if (snapshot.last_failed_binding_count != 0U) {
        return false;
    }

    return snapshot.last_required_binding_count == 0U;
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

int RequireBindingCapacityEntry(const InputReplaySnapshot &snapshot,
    InputDeviceId device,
    InputControlId control,
    InputActionId action,
    std::size_t binding_capacity,
    std::size_t binding_count,
    std::size_t required_binding_count,
    const char *message) {
    if (snapshot.last_failed_binding_device.value != device.value) {
        return Fail(message);
    }

    if (snapshot.last_failed_binding_control.value != control.value) {
        return Fail(message);
    }

    if (snapshot.last_failed_binding_action.value != action.value) {
        return Fail(message);
    }

    if (snapshot.last_failed_binding_capacity != binding_capacity) {
        return Fail(message);
    }

    if (snapshot.last_failed_binding_count != binding_count) {
        return Fail(message);
    }

    if (snapshot.last_required_binding_count != required_binding_count) {
        return Fail(message);
    }

    return 0;
}

int RequireBindingResultEntry(const InputBindingResult &result,
    InputDeviceId device,
    InputControlId control,
    std::size_t binding_capacity,
    std::size_t binding_count,
    std::size_t required_binding_count,
    const char *message) {
    if (result.device.value != device.value) {
        return Fail(message);
    }

    if (result.control.value != control.value) {
        return Fail(message);
    }

    if (result.binding_capacity != binding_capacity) {
        return Fail(message);
    }

    if (result.binding_count != binding_count) {
        return Fail(message);
    }

    if (result.required_binding_count != required_binding_count) {
        return Fail(message);
    }

    return 0;
}

int RequireBindingCapacityEntryCleared(const InputReplaySnapshot &snapshot, const char *message) {
    if (snapshot.last_failed_binding_device.value != InputDeviceId{}.value) {
        return Fail(message);
    }

    if (snapshot.last_failed_binding_control.value != InputControlId{}.value) {
        return Fail(message);
    }

    if (snapshot.last_failed_binding_action.value != InputActionId{}.value) {
        return Fail(message);
    }

    if (snapshot.last_failed_binding_capacity != 0U) {
        return Fail(message);
    }

    if (snapshot.last_failed_binding_count != 0U) {
        return Fail(message);
    }

    if (snapshot.last_required_binding_count != 0U) {
        return Fail(message);
    }

    return 0;
}

bool ReplayCapacityEntryIsClear(const InputReplaySnapshot &snapshot) {
    if (snapshot.last_failed_frame_index != 0U) {
        return false;
    }

    if (snapshot.last_failed_event_index != 0U) {
        return false;
    }

    if (snapshot.last_failed_event_type != InputEventType::ButtonPressed) {
        return false;
    }

    if (snapshot.last_failed_device.value != 0U) {
        return false;
    }

    if (snapshot.last_failed_control.value != 0U) {
        return false;
    }

    if (snapshot.last_failed_axis_value != 0) {
        return false;
    }

    if (snapshot.last_failed_event_capacity != 0U) {
        return false;
    }

    if (snapshot.last_failed_event_count != 0U) {
        return false;
    }

    return snapshot.last_required_event_count == 0U;
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

    if (snapshot.last_status != InputStatus::Success) {
        return Fail("binding registration did not record success status");
    }

    if (snapshot.last_apply_status != InputStatus::Success) {
        return Fail("binding registration changed apply status");
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

    if (after_snapshot.last_status != InputStatus::DuplicateBinding) {
        return Fail("duplicate control did not record last status");
    }

    if (after_snapshot.last_apply_status != before_snapshot.last_apply_status) {
        return Fail("duplicate control changed apply status");
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

    const auto after_snapshot = replay.Snapshot();
    if (after_snapshot.binding_count != before_snapshot.binding_count) {
        return Fail("binding overflow mutated binding count");
    }

    if (after_snapshot.last_status != InputStatus::CapacityExceeded) {
        return Fail("binding overflow did not record last status");
    }

    if (after_snapshot.last_apply_status != before_snapshot.last_apply_status) {
        return Fail("binding overflow changed apply status");
    }

    return 0;
}

int InputBindingCapacityOverflowReportsRejectedEntry() {
    InputReplay replay;
    const InputControlId first_control{0U};
    for (std::size_t index = 0U; index < MAX_INPUT_BINDINGS; ++index) {
        const std::uint32_t control_value = static_cast<std::uint32_t>(index);
        const InputControlId control{control_value};
        const InputBindingResult result = replay.RegisterActionBinding(DEVICE_A, control, ACTION_A);
        if (result.status != InputStatus::Success) {
            return Fail("binding entry setup failed before capacity");
        }
    }

    const InputReplaySnapshot before_snapshot = replay.Snapshot();
    const InputControlId overflow_control{999U};
    const std::size_t binding_capacity = MAX_INPUT_BINDINGS;
    const std::size_t binding_count = before_snapshot.binding_count;
    const std::size_t required_binding_count = MAX_INPUT_BINDINGS + 1U;
    const InputBindingResult overflow_result = replay.RegisterActionBinding(DEVICE_A, overflow_control, ACTION_B);
    if (overflow_result.status != InputStatus::CapacityExceeded) {
        return Fail("binding entry overflow returned wrong status");
    }

    if (overflow_result.action.value != ACTION_B.value) {
        return Fail("binding entry overflow returned wrong action");
    }

    const int overflow_result_entry = RequireBindingResultEntry(
        overflow_result,
        DEVICE_A,
        overflow_control,
        binding_capacity,
        binding_count,
        required_binding_count,
        "binding entry overflow result missed rejected binding");
    if (overflow_result_entry != 0) {
        return 1;
    }

    const InputReplaySnapshot overflow_snapshot = replay.Snapshot();
    if (overflow_snapshot.binding_count != before_snapshot.binding_count) {
        return Fail("binding entry overflow mutated binding count");
    }

    if (overflow_snapshot.last_status != InputStatus::CapacityExceeded) {
        return Fail("binding entry overflow did not record status");
    }

    const int overflow_snapshot_entry = RequireBindingCapacityEntry(
        overflow_snapshot,
        DEVICE_A,
        overflow_control,
        ACTION_B,
        binding_capacity,
        binding_count,
        required_binding_count,
        "binding entry overflow snapshot missed rejected binding");
    if (overflow_snapshot_entry != 0) {
        return 1;
    }

    const InputBindingResult duplicate_result = replay.RegisterActionBinding(DEVICE_A, first_control, ACTION_B);
    if (duplicate_result.status != InputStatus::DuplicateBinding) {
        return Fail("binding entry duplicate returned wrong status");
    }

    const InputReplaySnapshot duplicate_snapshot = replay.Snapshot();
    const int duplicate_clear_result = RequireBindingCapacityEntryCleared(
        duplicate_snapshot,
        "binding entry duplicate did not clear stale capacity entry");
    if (duplicate_clear_result != 0) {
        return 1;
    }

    const InputBindingResult second_overflow_result = replay.RegisterActionBinding(DEVICE_A, overflow_control, ACTION_B);
    if (second_overflow_result.status != InputStatus::CapacityExceeded) {
        return Fail("binding entry second overflow returned wrong status");
    }

    const InputBindingResult unknown_device_result =
        replay.RegisterActionBinding(UNKNOWN_DEVICE, overflow_control, ACTION_B);
    if (unknown_device_result.status != InputStatus::UnknownDeviceControl) {
        return Fail("binding entry unknown device returned wrong status");
    }

    const InputReplaySnapshot unknown_device_snapshot = replay.Snapshot();
    const int unknown_device_clear_result = RequireBindingCapacityEntryCleared(
        unknown_device_snapshot,
        "binding entry unknown device did not clear stale capacity entry");
    if (unknown_device_clear_result != 0) {
        return 1;
    }

    const InputBindingResult third_overflow_result = replay.RegisterActionBinding(DEVICE_A, overflow_control, ACTION_B);
    if (third_overflow_result.status != InputStatus::CapacityExceeded) {
        return Fail("binding entry third overflow returned wrong status");
    }

    const InputBindingResult unknown_action_result =
        replay.RegisterActionBinding(DEVICE_A, overflow_control, UNKNOWN_ACTION);
    if (unknown_action_result.status != InputStatus::UnknownAction) {
        return Fail("binding entry unknown action returned wrong status");
    }

    const InputReplaySnapshot unknown_action_snapshot = replay.Snapshot();
    const int unknown_action_clear_result = RequireBindingCapacityEntryCleared(
        unknown_action_snapshot,
        "binding entry unknown action did not clear stale capacity entry");
    if (unknown_action_clear_result != 0) {
        return 1;
    }

    for (std::size_t index = 0U; index < MAX_EVENTS_PER_FRAME; ++index) {
        const InputEvent event = ButtonPress(DEVICE_A, first_control);
        const InputStatus status = replay.RecordReplayEvent(0U, event);
        if (status != InputStatus::Success) {
            return Fail("binding entry event setup failed before capacity");
        }
    }

    const InputBindingResult fourth_overflow_result = replay.RegisterActionBinding(DEVICE_A, overflow_control, ACTION_B);
    if (fourth_overflow_result.status != InputStatus::CapacityExceeded) {
        return Fail("binding entry fourth overflow returned wrong status");
    }

    const InputEvent capacity_event = ButtonRelease(DEVICE_A, first_control);
    const InputStatus capacity_event_status = replay.RecordReplayEvent(0U, capacity_event);
    if (capacity_event_status != InputStatus::CapacityExceeded) {
        return Fail("binding entry replay capacity returned wrong status");
    }

    const InputReplaySnapshot event_capacity_snapshot = replay.Snapshot();
    const int event_capacity_clear_result = RequireBindingCapacityEntryCleared(
        event_capacity_snapshot,
        "binding entry replay capacity did not clear stale capacity entry");
    if (event_capacity_clear_result != 0) {
        return 1;
    }

    const InputBindingResult final_overflow_result = replay.RegisterActionBinding(DEVICE_A, overflow_control, ACTION_B);
    if (final_overflow_result.status != InputStatus::CapacityExceeded) {
        return Fail("binding entry final overflow returned wrong status");
    }

    const InputEvent success_event = ButtonPress(DEVICE_A, first_control);
    const InputStatus success_event_status = replay.RecordReplayEvent(1U, success_event);
    if (success_event_status != InputStatus::Success) {
        return Fail("binding entry success event failed");
    }

    const InputReplaySnapshot success_snapshot = replay.Snapshot();
    const int success_clear_result = RequireBindingCapacityEntryCleared(
        success_snapshot,
        "binding entry success did not clear stale capacity entry");
    if (success_clear_result != 0) {
        return 1;
    }

    return 0;
}

int InputReplayFrameAppliesButtonPressAndRelease() {
    InputReplay replay;
    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    if (replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A)) != InputStatus::Success) {
        return Fail("button press event record failed");
    }

    const auto event_snapshot = replay.Snapshot();
    if (event_snapshot.last_status != InputStatus::Success) {
        return Fail("event record did not record success status");
    }

    if (event_snapshot.last_apply_status != InputStatus::Success) {
        return Fail("event record changed apply status");
    }

    if (replay.RecordReplayEvent(1U, ButtonRelease(DEVICE_A, CONTROL_A)) != InputStatus::Success) {
        return Fail("button release event record failed");
    }

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

    const auto after_snapshot = replay.Snapshot();
    if (after_snapshot.accepted_event_count != before_snapshot.accepted_event_count) {
        return Fail("invalid axis mutated accepted event count");
    }

    if (after_snapshot.last_status != InputStatus::InvalidAxisValue) {
        return Fail("invalid axis did not record last status");
    }

    if (after_snapshot.last_apply_status != before_snapshot.last_apply_status) {
        return Fail("invalid axis changed apply status");
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

    const auto after_snapshot = replay.Snapshot();
    if (after_snapshot.accepted_event_count != before_snapshot.accepted_event_count) {
        return Fail("invalid event mutated accepted count");
    }

    if (after_snapshot.last_status != InputStatus::InvalidEvent) {
        return Fail("invalid event did not record last status");
    }

    if (after_snapshot.last_apply_status != before_snapshot.last_apply_status) {
        return Fail("invalid event changed apply status");
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

    const auto unknown_device_snapshot = replay.Snapshot();
    if (unknown_device_snapshot.last_status != InputStatus::UnknownDeviceControl) {
        return Fail("unknown device did not record last status");
    }

    if (unknown_device_snapshot.last_apply_status != InputStatus::Success) {
        return Fail("unknown device changed apply status");
    }

    if (!RegisterPrimaryBinding(replay)) {
        return Fail("binding failed");
    }

    const auto binding_snapshot = replay.Snapshot();
    if (binding_snapshot.last_status != InputStatus::Success) {
        return Fail("binding did not restore success status");
    }

    if (replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, UNKNOWN_CONTROL)) != InputStatus::UnknownDeviceControl) {
        return Fail("unknown control did not return explicit status");
    }

    const auto unknown_control_snapshot = replay.Snapshot();
    if (unknown_control_snapshot.last_status != InputStatus::UnknownDeviceControl) {
        return Fail("unknown control did not record last status");
    }

    if (unknown_control_snapshot.last_apply_status != binding_snapshot.last_apply_status) {
        return Fail("unknown control changed apply status");
    }

    if (replay.QueryAction(UNKNOWN_ACTION).status != InputStatus::UnknownAction) {
        return Fail("unknown action did not return explicit status");
    }

    if (replay.Snapshot().last_status != unknown_control_snapshot.last_status) {
        return Fail("action query changed replay last status");
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

    const auto after_snapshot = replay.Snapshot();
    if (after_snapshot.accepted_event_count != before_snapshot.accepted_event_count) {
        return Fail("event overflow mutated accepted event count");
    }

    if (after_snapshot.last_status != InputStatus::CapacityExceeded) {
        return Fail("event overflow did not record last status");
    }

    if (after_snapshot.last_apply_status != before_snapshot.last_apply_status) {
        return Fail("event overflow changed apply status");
    }

    return 0;
}

int InputReplayEventCapacityFailureReportsEntry() {
    InputReplay replay;
    const std::size_t invalid_frame_index = MAX_REPLAY_FRAMES;
    const InputEvent invalid_frame_event = Axis(DEVICE_B, CONTROL_B, AXIS_NEGATIVE);
    InputStatus status = replay.RecordReplayEvent(invalid_frame_index, invalid_frame_event);
    if (status != InputStatus::CapacityExceeded) {
        return Fail("invalid frame did not report capacity");
    }

    const std::size_t replay_event_capacity = MAX_REPLAY_FRAMES * MAX_EVENTS_PER_FRAME;
    const std::size_t required_replay_event_count = replay_event_capacity + 1U;
    InputReplaySnapshot snapshot = replay.Snapshot();
    if (snapshot.last_failed_frame_index != invalid_frame_index) {
        return Fail("invalid frame capacity index mismatch");
    }

    if (snapshot.last_failed_event_index != 0U) {
        return Fail("invalid frame event index mismatch");
    }

    if (snapshot.last_failed_event_type != invalid_frame_event.type ||
        snapshot.last_failed_device.value != invalid_frame_event.device.value ||
        snapshot.last_failed_control.value != invalid_frame_event.control.value ||
        snapshot.last_failed_axis_value != invalid_frame_event.axis_value) {
        return Fail("invalid frame capacity event mismatch");
    }

    if (snapshot.last_failed_event_capacity != replay_event_capacity ||
        snapshot.last_failed_event_count != 0U ||
        snapshot.last_required_event_count != required_replay_event_count) {
        return Fail("invalid frame capacity counts mismatch");
    }

    const InputEvent invalid_event{DEVICE_A, CONTROL_A, static_cast<InputEventType>(99), 0};
    status = replay.RecordReplayEvent(0U, invalid_event);
    if (status != InputStatus::InvalidEvent) {
        return Fail("invalid event did not clear through non-capacity failure");
    }

    snapshot = replay.Snapshot();
    if (!ReplayCapacityEntryIsClear(snapshot)) {
        return Fail("invalid event left stale capacity entry");
    }

    InputReplay full_replay;
    if (!RegisterPrimaryBinding(full_replay)) {
        return Fail("capacity entry binding failed");
    }

    constexpr std::size_t FULL_FRAME_INDEX = 1U;
    for (std::size_t index = 0U; index < MAX_EVENTS_PER_FRAME; ++index) {
        status = full_replay.RecordReplayEvent(FULL_FRAME_INDEX, ButtonPress(DEVICE_A, CONTROL_A));
        if (status != InputStatus::Success) {
            return Fail("frame fill failed before capacity");
        }
    }

    const InputEvent overflow_event = ButtonRelease(DEVICE_A, CONTROL_A);
    status = full_replay.RecordReplayEvent(FULL_FRAME_INDEX, overflow_event);
    if (status != InputStatus::CapacityExceeded) {
        return Fail("full frame did not report capacity");
    }

    if (full_replay.EventCountForFrame(FULL_FRAME_INDEX) != MAX_EVENTS_PER_FRAME) {
        return Fail("full frame capacity mutated replay frame");
    }

    snapshot = full_replay.Snapshot();
    if (snapshot.last_failed_frame_index != FULL_FRAME_INDEX ||
        snapshot.last_failed_event_index != MAX_EVENTS_PER_FRAME ||
        snapshot.last_failed_event_type != overflow_event.type ||
        snapshot.last_failed_device.value != overflow_event.device.value ||
        snapshot.last_failed_control.value != overflow_event.control.value) {
        return Fail("full frame capacity entry mismatch");
    }

    if (snapshot.last_failed_event_capacity != MAX_EVENTS_PER_FRAME ||
        snapshot.last_failed_event_count != MAX_EVENTS_PER_FRAME ||
        snapshot.last_required_event_count != MAX_EVENTS_PER_FRAME + 1U) {
        return Fail("full frame capacity counts mismatch");
    }

    const std::int32_t invalid_axis_value = AXIS_MAX_VALUE + 1;
    const InputEvent invalid_axis_event = Axis(DEVICE_A, CONTROL_A, invalid_axis_value);
    status = full_replay.RecordReplayEvent(FULL_FRAME_INDEX, invalid_axis_event);
    if (status != InputStatus::InvalidAxisValue) {
        return Fail("invalid axis did not win over capacity");
    }

    snapshot = full_replay.Snapshot();
    if (!ReplayCapacityEntryIsClear(snapshot)) {
        return Fail("invalid axis left stale capacity entry");
    }

    status = full_replay.RecordReplayEvent(MAX_REPLAY_FRAMES, overflow_event);
    if (status != InputStatus::CapacityExceeded) {
        return Fail("second capacity setup failed");
    }

    status = full_replay.RecordReplayEvent(FULL_FRAME_INDEX, ButtonPress(DEVICE_A, UNKNOWN_CONTROL));
    if (status != InputStatus::UnknownDeviceControl) {
        return Fail("unknown control did not win over capacity");
    }

    snapshot = full_replay.Snapshot();
    if (!ReplayCapacityEntryIsClear(snapshot)) {
        return Fail("unknown control left stale capacity entry");
    }

    status = full_replay.RecordReplayEvent(MAX_REPLAY_FRAMES, overflow_event);
    if (status != InputStatus::CapacityExceeded) {
        return Fail("third capacity setup failed");
    }

    status = full_replay.RecordReplayEvent(0U, ButtonPress(DEVICE_A, CONTROL_A));
    if (status != InputStatus::Success) {
        return Fail("success after capacity failed");
    }

    snapshot = full_replay.Snapshot();
    if (!ReplayCapacityEntryIsClear(snapshot)) {
        return Fail("success left stale capacity entry");
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

    if (replay.ResetFrameState() != InputStatus::Success) {
        return Fail("reset did not return success");
    }

    const auto state = replay.QueryAction(ACTION_A).state;
    if (!state.is_pressed) {
        return Fail("reset cleared pressed state");
    }

    if (state.changed_this_frame) {
        return Fail("reset did not clear changed state");
    }

    const auto reset_snapshot = replay.Snapshot();
    if (reset_snapshot.last_status != InputStatus::Success) {
        return Fail("reset did not record success status");
    }

    if (reset_snapshot.last_apply_status != InputStatus::Success) {
        return Fail("reset changed apply status");
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

int InputBridgeDescDefaultValuesAreBounded() {
    const InputBridgeDesc desc{};
    if (desc.backend != InputBackendKind::NativeMessage) {
        return Fail("bridge default backend is not native message");
    }

    if (desc.event_capacity < InputBridgeDesc::MIN_EVENT_CAPACITY) {
        return Fail("bridge default capacity is below minimum");
    }

    if (desc.event_capacity > InputBridgeDesc::MAX_EVENT_CAPACITY) {
        return Fail("bridge default capacity exceeds maximum");
    }

    if (desc.focus_policy != InputFocusPolicy::RejectWhenUnfocused) {
        return Fail("bridge default focus policy changed");
    }

    if (desc.gamepad_device.value != 2U) {
        return Fail("bridge default gamepad device changed");
    }

    return 0;
}

int InputBridgePublicContractUsesValueTypes() {
    if (!std::is_standard_layout_v<InputBridgeDesc>) {
        return Fail("bridge desc is not standard layout");
    }

    if (!std::is_trivially_copyable_v<InputBridgeDesc>) {
        return Fail("bridge desc is not trivially copyable");
    }

    if (!std::is_standard_layout_v<InputBridgeEvent>) {
        return Fail("bridge event is not standard layout");
    }

    if (!std::is_trivially_copyable_v<InputBridgeEvent>) {
        return Fail("bridge event is not trivially copyable");
    }

    if (!std::is_standard_layout_v<InputBridgeSnapshot>) {
        return Fail("bridge snapshot is not standard layout");
    }

    if (!std::is_trivially_copyable_v<InputBridgeSnapshot>) {
        return Fail("bridge snapshot is not trivially copyable");
    }

    if (!std::is_standard_layout_v<InputGamepadState>) {
        return Fail("gamepad state is not standard layout");
    }

    if (!std::is_trivially_copyable_v<InputGamepadState>) {
        return Fail("gamepad state is not trivially copyable");
    }

    if (std::is_copy_constructible_v<InputBridge>) {
        return Fail("bridge owner is copy constructible");
    }

    if (std::is_copy_assignable_v<InputBridge>) {
        return Fail("bridge owner is copy assignable");
    }

    return 0;
}

int InputBridgeInitializeRejectsUnsupportedBackend() {
    InputBridgeDesc desc{};
    desc.backend = InputBackendKind::Replay;

    InputBridge bridge;
    if (bridge.Initialize(desc) != InputStatus::UnsupportedBackend) {
        return Fail("bridge did not reject replay backend");
    }

    const auto snapshot = bridge.Snapshot();
    if (snapshot.unsupported_backend_count != 1U) {
        return Fail("unsupported backend counter did not update");
    }

    if (snapshot.initialized) {
        return Fail("unsupported backend initialized bridge");
    }

    return 0;
}

int InputBridgeSubmitAndDrainRecordsKeyboardMouseWheel() {
    InputBridge bridge;
    InputBridgeDesc desc{};
    desc.event_capacity = 3U;
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    if (bridge.SubmitEvent(BridgeKey(InputBridgeEventType::KeyPressed, 65U)) != InputStatus::Success) {
        return Fail("key bridge event failed");
    }

    if (bridge.SubmitEvent(BridgeMouseMove(10, -20)) != InputStatus::Success) {
        return Fail("mouse move bridge event failed");
    }

    if (bridge.SubmitEvent(BridgeMouseWheel(120)) != InputStatus::Success) {
        return Fail("wheel bridge event failed");
    }

    std::array<InputBridgeEvent, 3U> events{};
    std::size_t event_count = 0U;
    if (bridge.DrainEvents(events.data(), events.size(), event_count) != InputStatus::Success) {
        return Fail("bridge drain failed");
    }

    if (event_count != events.size()) {
        return Fail("bridge drain count mismatch");
    }

    if (events[0].type != InputBridgeEventType::KeyPressed) {
        return Fail("bridge key event type mismatch");
    }

    if (events[1].type != InputBridgeEventType::MouseMoved) {
        return Fail("bridge mouse move event type mismatch");
    }

    if (events[2].wheel_delta != 120) {
        return Fail("bridge wheel delta mismatch");
    }

    const auto snapshot = bridge.Snapshot();
    if (snapshot.accepted_event_count != 3U) {
        return Fail("bridge accepted count mismatch");
    }

    if (snapshot.drained_event_count != 3U) {
        return Fail("bridge drained count mismatch");
    }

    if (snapshot.required_output_event_count != 0U) {
        return Fail("bridge drain left required output count");
    }

    if (snapshot.queued_event_count != 0U) {
        return Fail("bridge queue was not drained");
    }

    return 0;
}

int InputBridgeFocusLostRejectsInputAndTracksCounters() {
    InputBridge bridge;
    InputBridgeDesc desc{};
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    if (bridge.SetFocus(false) != InputStatus::Success) {
        return Fail("bridge focus lost update failed");
    }

    if (bridge.SubmitEvent(BridgeKey(InputBridgeEventType::KeyPressed, 65U)) != InputStatus::FocusLost) {
        return Fail("bridge did not reject input while unfocused");
    }

    if (bridge.SetFocus(true) != InputStatus::Success) {
        return Fail("bridge focus gained update failed");
    }

    if (bridge.SubmitEvent(BridgeKey(InputBridgeEventType::KeyPressed, 65U)) != InputStatus::Success) {
        return Fail("bridge did not accept input after focus gained");
    }

    const auto snapshot = bridge.Snapshot();
    if (snapshot.focus_lost_count != 1U) {
        return Fail("bridge focus lost counter mismatch");
    }

    if (snapshot.focus_gained_count != 1U) {
        return Fail("bridge focus gained counter mismatch");
    }

    if (snapshot.rejected_event_count != 1U) {
        return Fail("bridge rejected count mismatch after focus lost");
    }

    return 0;
}

int InputBridgeCapacityOverflowDoesNotGrow() {
    InputBridge bridge;
    InputBridgeDesc desc{};
    desc.event_capacity = 1U;
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    if (bridge.SubmitEvent(BridgeKey(InputBridgeEventType::KeyPressed, 65U)) != InputStatus::Success) {
        return Fail("bridge first event failed");
    }

    if (bridge.SubmitEvent(BridgeKey(InputBridgeEventType::KeyReleased, 65U)) != InputStatus::CapacityExceeded) {
        return Fail("bridge overflow did not return capacity status");
    }

    const auto snapshot = bridge.Snapshot();
    if (snapshot.event_capacity != 1U) {
        return Fail("bridge capacity changed");
    }

    if (snapshot.max_queued_event_count != 1U) {
        return Fail("bridge max queued count changed");
    }

    if (snapshot.overflow_count != 1U) {
        return Fail("bridge overflow counter mismatch");
    }

    if (snapshot.accepted_event_count != 1U) {
        return Fail("bridge accepted count changed after overflow");
    }

    return 0;
}

int InputBridgeDrainRejectsSmallOutputWithoutMutation() {
    InputBridge bridge;
    InputBridgeDesc desc{};
    desc.event_capacity = 2U;
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    bridge.SubmitEvent(BridgeKey(InputBridgeEventType::KeyPressed, 65U));
    bridge.SubmitEvent(BridgeKey(InputBridgeEventType::KeyReleased, 65U));

    std::array<InputBridgeEvent, 1U> small_events{};
    std::size_t small_count = 0U;
    if (bridge.DrainEvents(small_events.data(), small_events.size(), small_count) != InputStatus::OutputBufferFull) {
        return Fail("bridge did not reject undersized drain buffer");
    }

    if (small_count != 0U) {
        return Fail("undersized drain wrote event count");
    }

    const auto rejected_snapshot = bridge.Snapshot();
    if (rejected_snapshot.queued_event_count != 2U) {
        return Fail("undersized drain mutated queue");
    }

    if (rejected_snapshot.required_output_event_count != 2U) {
        return Fail("undersized drain did not record required output count");
    }

    if (rejected_snapshot.last_status != InputStatus::OutputBufferFull) {
        return Fail("undersized drain did not record last status");
    }

    std::array<InputBridgeEvent, 2U> events{};
    std::size_t event_count = 0U;
    if (bridge.DrainEvents(events.data(), events.size(), event_count) != InputStatus::Success) {
        return Fail("bridge full drain failed after undersized drain");
    }

    if (event_count != 2U) {
        return Fail("bridge full drain count mismatch");
    }

    const auto drained_snapshot = bridge.Snapshot();
    if (drained_snapshot.required_output_event_count != 0U) {
        return Fail("full drain did not clear required output count");
    }

    return 0;
}

int InputBridgeGamepadUnavailableStateIsExplicit() {
    InputBridge bridge;
    InputBridgeDesc desc{};
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    const InputGamepadState state = UnavailableGamepadState(desc.gamepad_device);
    if (bridge.SubmitGamepadState(state) != InputStatus::DeviceUnavailable) {
        return Fail("gamepad unavailable did not return explicit status");
    }

    const auto snapshot = bridge.Snapshot();
    if (snapshot.unavailable_count != 1U) {
        return Fail("gamepad unavailable counter mismatch");
    }

    if (snapshot.gamepad_unavailable_poll_count != 1U) {
        return Fail("gamepad unavailable poll counter mismatch");
    }

    if (snapshot.gamepad_connection != InputGamepadConnection::Unavailable) {
        return Fail("gamepad unavailable connection state mismatch");
    }

    if (snapshot.accepted_event_count != 0U) {
        return Fail("gamepad unavailable mutated accepted count");
    }

    return 0;
}

int InputBridgeGamepadConnectedStateQueuesButtonAndAxisEvents() {
    InputBridge bridge;
    InputBridgeDesc desc{};
    desc.event_capacity = 4U;
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    InputGamepadState state = GamepadState(desc.gamepad_device, 1U);
    state.buttons = GAMEPAD_BUTTON_A;
    state.right_trigger = 64U;
    state.left_thumb_x = 1234;
    if (bridge.SubmitGamepadState(state) != InputStatus::Success) {
        return Fail("gamepad connected state submit failed");
    }

    std::array<InputBridgeEvent, 3U> events{};
    std::size_t event_count = 0U;
    if (bridge.DrainEvents(events.data(), events.size(), event_count) != InputStatus::Success) {
        return Fail("gamepad connected drain failed");
    }

    if (event_count != events.size()) {
        return Fail("gamepad connected event count mismatch");
    }

    if (events[0U].type != InputBridgeEventType::GamepadButtonPressed) {
        return Fail("gamepad connected did not queue button press");
    }

    if (events[0U].raw_code != GAMEPAD_BUTTON_A) {
        return Fail("gamepad button raw code mismatch");
    }

    if (events[1U].type != InputBridgeEventType::GamepadAxisMoved) {
        return Fail("gamepad connected did not queue trigger axis");
    }

    if (events[1U].control.value != GAMEPAD_RIGHT_TRIGGER_CONTROL) {
        return Fail("gamepad trigger control mismatch");
    }

    if (events[2U].control.value != GAMEPAD_LEFT_THUMB_X_CONTROL) {
        return Fail("gamepad thumb control mismatch");
    }

    const auto snapshot = bridge.Snapshot();
    if (snapshot.gamepad_connection != InputGamepadConnection::Connected) {
        return Fail("gamepad connected snapshot state mismatch");
    }

    if (snapshot.gamepad_event_count != 3U) {
        return Fail("gamepad connected event counter mismatch");
    }

    if (snapshot.last_gamepad_packet_number != 1U) {
        return Fail("gamepad packet number was not tracked");
    }

    return 0;
}

int InputBridgeGamepadRepeatedPacketDoesNotQueueDuplicateEvents() {
    InputBridge bridge;
    InputBridgeDesc desc{};
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    InputGamepadState state = GamepadState(desc.gamepad_device, 7U);
    state.buttons = GAMEPAD_BUTTON_A;
    if (bridge.SubmitGamepadState(state) != InputStatus::Success) {
        return Fail("initial gamepad state submit failed");
    }

    std::array<InputBridgeEvent, 1U> first_events{};
    std::size_t first_count = 0U;
    if (bridge.DrainEvents(first_events.data(), first_events.size(), first_count) != InputStatus::Success) {
        return Fail("initial gamepad drain failed");
    }

    if (first_count != 1U) {
        return Fail("initial gamepad event count mismatch");
    }

    if (bridge.SubmitGamepadState(state) != InputStatus::Success) {
        return Fail("repeated gamepad state submit failed");
    }

    std::array<InputBridgeEvent, 1U> repeated_events{};
    std::size_t repeated_count = 0U;
    if (bridge.DrainEvents(repeated_events.data(), repeated_events.size(), repeated_count) != InputStatus::Success) {
        return Fail("repeated gamepad drain failed");
    }

    if (repeated_count != 0U) {
        return Fail("repeated packet queued duplicate event");
    }

    const auto snapshot = bridge.Snapshot();
    if (snapshot.gamepad_poll_count != 2U) {
        return Fail("repeated packet poll counter mismatch");
    }

    if (snapshot.gamepad_event_count != 1U) {
        return Fail("repeated packet event counter mismatch");
    }

    return 0;
}

int InputBridgeGamepadCapacityOverflowDoesNotQueuePartialEvents() {
    InputBridge bridge;
    InputBridgeDesc desc{};
    desc.event_capacity = 2U;
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    if (bridge.SubmitEvent(BridgeKey(InputBridgeEventType::KeyPressed, 65U)) != InputStatus::Success) {
        return Fail("bridge initial key submit failed");
    }

    InputGamepadState state = GamepadState(desc.gamepad_device, 2U);
    state.buttons = static_cast<std::uint16_t>(GAMEPAD_BUTTON_A | GAMEPAD_BUTTON_B);
    state.right_trigger = 64U;
    if (bridge.SubmitGamepadState(state) != InputStatus::CapacityExceeded) {
        return Fail("gamepad capacity overflow did not reject");
    }

    const auto snapshot = bridge.Snapshot();
    if (!GamepadCapacityIdentityMatches(snapshot, state, 2U, 1U, 4U)) {
        return Fail("gamepad overflow did not record rejected state identity");
    }

    if (snapshot.queued_event_count != 1U) {
        return Fail("gamepad overflow mutated queued event count");
    }

    if (snapshot.overflow_count != 1U) {
        return Fail("gamepad overflow counter mismatch");
    }

    if (snapshot.gamepad_event_count != 0U) {
        return Fail("gamepad overflow mutated event counter");
    }

    InputBridgeEvent invalid_event{};
    if (bridge.SubmitEvent(invalid_event) != InputStatus::InvalidEvent) {
        return Fail("invalid bridge event did not return invalid status");
    }

    if (!GamepadCapacityIdentityIsClear(bridge.Snapshot())) {
        return Fail("invalid event did not clear gamepad capacity identity");
    }

    std::array<InputBridgeEvent, 2U> events{};
    std::size_t event_count = 0U;
    if (bridge.DrainEvents(events.data(), events.size(), event_count) != InputStatus::Success) {
        return Fail("gamepad overflow drain failed");
    }

    if (event_count != 1U) {
        return Fail("gamepad overflow drain count mismatch");
    }

    if (events[0U].type != InputBridgeEventType::KeyPressed) {
        return Fail("gamepad overflow replaced existing queued event");
    }

    if (!GamepadCapacityIdentityIsClear(bridge.Snapshot())) {
        return Fail("drain did not clear gamepad capacity identity");
    }

    return 0;
}

int InputBridgeXInputPollGamepadCapacityOverflowRecordsRejectedState() {
    ScopedNativeGamepadPollFunction poll_function(PollNativeCapacityState);
    InputBridge bridge;
    InputBridgeDesc desc{};
    desc.event_capacity = 2U;
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    const InputStatus poll_status = bridge.PollGamepad(0U);
    if (poll_status != InputStatus::CapacityExceeded) {
        return Fail("xinput gamepad capacity did not reject");
    }

    InputGamepadState expected_state = GamepadState(desc.gamepad_device, 44U);
    expected_state.buttons = static_cast<std::uint16_t>(GAMEPAD_BUTTON_A | GAMEPAD_BUTTON_B);
    expected_state.left_trigger = 9U;
    expected_state.right_trigger = 10U;
    expected_state.left_thumb_x = 111;
    expected_state.left_thumb_y = -112;
    const auto snapshot = bridge.Snapshot();
    if (!GamepadCapacityIdentityMatches(snapshot, expected_state, 2U, 0U, 6U)) {
        return Fail("xinput gamepad capacity did not record rejected state");
    }

    if (snapshot.gamepad_connected_poll_count != 1U) {
        return Fail("xinput gamepad capacity connected poll counter mismatch");
    }

    if (snapshot.gamepad_event_count != 0U) {
        return Fail("xinput gamepad capacity queued partial events");
    }

    return 0;
}

int InputBridgeXInputPollUnavailableReturnsDeviceUnavailable() {
    ScopedNativeGamepadPollFunction poll_function(PollNativeUnavailable);
    InputBridge bridge;
    InputBridgeDesc desc{};
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    const InputStatus poll_status = bridge.PollGamepad(0U);
    if (poll_status != InputStatus::DeviceUnavailable) {
        return Fail("xinput unavailable did not return device unavailable");
    }

    const auto snapshot = bridge.Snapshot();
    if (snapshot.gamepad_poll_count != 1U) {
        return Fail("xinput unavailable poll count mismatch");
    }

    if (snapshot.gamepad_unavailable_poll_count != 1U) {
        return Fail("xinput unavailable counter mismatch");
    }

    if (snapshot.gamepad_connection != InputGamepadConnection::Unavailable) {
        return Fail("xinput unavailable connection mismatch");
    }

    if (snapshot.last_status != InputStatus::DeviceUnavailable) {
        return Fail("xinput unavailable last status mismatch");
    }

    return 0;
}

int InputBridgeXInputPollConnectedQueuesButtonAxisAndPacket() {
    ScopedNativeGamepadPollFunction poll_function(PollNativeConnected);
    InputBridge bridge;
    InputBridgeDesc desc{};
    desc.event_capacity = 4U;
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    if (bridge.PollGamepad(0U) != InputStatus::Success) {
        return Fail("xinput connected poll failed");
    }

    std::array<InputBridgeEvent, 3U> events{};
    std::size_t event_count = 0U;
    if (bridge.DrainEvents(events.data(), events.size(), event_count) != InputStatus::Success) {
        return Fail("xinput connected drain failed");
    }

    if (event_count != events.size()) {
        return Fail("xinput connected event count mismatch");
    }

    if (events[0U].type != InputBridgeEventType::GamepadButtonPressed) {
        return Fail("xinput connected did not queue button event");
    }

    if (events[0U].raw_code != GAMEPAD_BUTTON_A) {
        return Fail("xinput connected button raw code mismatch");
    }

    if (events[1U].control.value != GAMEPAD_RIGHT_TRIGGER_CONTROL) {
        return Fail("xinput connected trigger control mismatch");
    }

    if (events[1U].axis_value != 64) {
        return Fail("xinput connected trigger value mismatch");
    }

    if (events[2U].control.value != GAMEPAD_LEFT_THUMB_X_CONTROL) {
        return Fail("xinput connected thumb control mismatch");
    }

    if (events[2U].axis_value != -1234) {
        return Fail("xinput connected thumb value mismatch");
    }

    const auto first_snapshot = bridge.Snapshot();
    if (first_snapshot.last_gamepad_packet_number != 31U) {
        return Fail("xinput connected packet mismatch");
    }

    if (bridge.PollGamepad(0U) != InputStatus::Success) {
        return Fail("xinput repeated packet poll failed");
    }

    std::array<InputBridgeEvent, 1U> repeated_events{};
    std::size_t repeated_count = 0U;
    if (bridge.DrainEvents(repeated_events.data(), repeated_events.size(), repeated_count) != InputStatus::Success) {
        return Fail("xinput repeated packet drain failed");
    }

    if (repeated_count != 0U) {
        return Fail("xinput repeated packet queued duplicate events");
    }

    const auto second_snapshot = bridge.Snapshot();
    if (second_snapshot.gamepad_connected_poll_count != 2U) {
        return Fail("xinput connected poll counter mismatch");
    }

    if (second_snapshot.gamepad_event_count != 3U) {
        return Fail("xinput connected event counter mismatch");
    }

    return 0;
}

int InputBridgeXInputPollBackendErrorReturnsExplicitStatus() {
    ScopedNativeGamepadPollFunction poll_function(PollNativeBackendError);
    InputBridge bridge;
    InputBridgeDesc desc{};
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    if (bridge.PollGamepad(0U) != InputStatus::BackendError) {
        return Fail("xinput backend error did not return explicit status");
    }

    const auto snapshot = bridge.Snapshot();
    if (snapshot.last_status != InputStatus::BackendError) {
        return Fail("xinput backend error last status mismatch");
    }

    if (snapshot.rejected_event_count != 1U) {
        return Fail("xinput backend error reject count mismatch");
    }

    if (snapshot.gamepad_poll_count != 0U) {
        return Fail("xinput backend error mutated gamepad poll count");
    }

    if (snapshot.accepted_event_count != 0U) {
        return Fail("xinput backend error mutated accepted events");
    }

    return 0;
}

int InputBridgeXInputPollInvalidUserIndexReturnsExplicitStatus() {
    ScopedNativeGamepadPollFunction poll_function(PollNativeConnected);
    InputBridge bridge;
    InputBridgeDesc desc{};
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    if (bridge.PollGamepad(MAX_GAMEPAD_DEVICES) != InputStatus::InvalidUserIndex) {
        return Fail("xinput invalid user index did not return explicit status");
    }

    const auto snapshot = bridge.Snapshot();
    if (snapshot.last_status != InputStatus::InvalidUserIndex) {
        return Fail("xinput invalid user index last status mismatch");
    }

    if (snapshot.rejected_event_count != 1U) {
        return Fail("xinput invalid user index reject count mismatch");
    }

    if (snapshot.gamepad_poll_count != 0U) {
        return Fail("xinput invalid user index called backend poll");
    }

    return 0;
}

int InputBridgeNoUiGameOrReportDispatch() {
    InputBridge bridge;
    InputBridgeDesc desc{};
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    if (bridge.SubmitEvent(BridgeMouseWheel(120)) != InputStatus::Success) {
        return Fail("minimal bridge path failed");
    }

    std::array<InputBridgeEvent, 1U> events{};
    std::size_t event_count = 0U;
    if (bridge.DrainEvents(events.data(), events.size(), event_count) != InputStatus::Success) {
        return Fail("minimal bridge drain failed");
    }

    if (event_count != 1U) {
        return Fail("minimal bridge drain count mismatch");
    }

    if (events[0].type != InputBridgeEventType::MouseWheel) {
        return Fail("minimal bridge event type mismatch");
    }

    return 0;
}

int InputCommandMapperKeyboardBuildsFrameCommandSnapshot() {
    InputCommandMapper mapper;
    if (!RegisterCommandContext(mapper)) {
        return Fail("command context registration failed");
    }

    InputCommandBinding invalid_binding = ButtonCommandBinding(CONTEXT_A, DEVICE_A, CONTROL_B, ACTION_B);
    invalid_binding.value_kind = static_cast<InputCommandValueKind>(99);
    if (mapper.RegisterBinding(invalid_binding) != InputStatus::InvalidDescriptor) {
        return Fail("invalid command value kind did not return explicit status");
    }

    if (mapper.RegisterBinding(ButtonCommandBinding(CONTEXT_A, DEVICE_A, CONTROL_A, ACTION_A)) != InputStatus::Success) {
        return Fail("keyboard command binding failed");
    }

    std::array<InputEvent, 1U> press_events{ButtonPress(DEVICE_A, CONTROL_A)};
    InputCommandSnapshot press_snapshot{};
    const InputStatus press_status = mapper.BuildSnapshot(
        42U,
        std::span<const InputEvent>(press_events.data(), press_events.size()),
        &press_snapshot);
    if (press_status != InputStatus::Success) {
        return Fail("keyboard command press snapshot failed");
    }

    if (press_snapshot.frame_index != 42U || press_snapshot.command_count != 1U) {
        return Fail("keyboard command snapshot metadata mismatch");
    }

    const auto &press_record = press_snapshot.commands[0U];
    if (!press_record.pressed_this_frame || !press_record.held || press_record.released_this_frame) {
        return Fail("keyboard press command state mismatch");
    }

    InputCommandSnapshot held_snapshot{};
    if (mapper.BuildSnapshot(43U, std::span<const InputEvent>(), &held_snapshot) != InputStatus::Success) {
        return Fail("keyboard held command snapshot failed");
    }

    const auto &held_record = held_snapshot.commands[0U];
    if (held_record.pressed_this_frame || !held_record.held || held_record.released_this_frame) {
        return Fail("keyboard held command state mismatch");
    }

    std::array<InputEvent, 1U> release_events{ButtonRelease(DEVICE_A, CONTROL_A)};
    InputCommandSnapshot release_snapshot{};
    if (mapper.BuildSnapshot(
        44U,
        std::span<const InputEvent>(release_events.data(), release_events.size()),
        &release_snapshot) != InputStatus::Success) {
        return Fail("keyboard release command snapshot failed");
    }

    const auto &release_record = release_snapshot.commands[0U];
    if (release_record.pressed_this_frame || release_record.held || !release_record.released_this_frame) {
        return Fail("keyboard release command state mismatch");
    }

    return 0;
}

int InputCommandMapperXInputStyleAxisBuildsCommandSnapshot() {
    InputCommandMapper mapper;
    if (!RegisterCommandContext(mapper)) {
        return Fail("command context registration failed");
    }

    const InputControlId gamepad_axis{GAMEPAD_LEFT_THUMB_X_CONTROL};
    if (mapper.RegisterBinding(AxisCommandBinding(CONTEXT_A, DEVICE_B, gamepad_axis, ACTION_B)) != InputStatus::Success) {
        return Fail("xinput style axis command binding failed");
    }

    std::array<InputEvent, 1U> axis_events{Axis(DEVICE_B, gamepad_axis, AXIS_NEGATIVE)};
    InputCommandSnapshot snapshot{};
    const InputStatus status = mapper.BuildSnapshot(
        7U,
        std::span<const InputEvent>(axis_events.data(), axis_events.size()),
        &snapshot);
    if (status != InputStatus::Success) {
        return Fail("xinput style axis snapshot failed");
    }

    if (snapshot.command_count != 1U) {
        return Fail("xinput style axis command count mismatch");
    }

    const auto &record = snapshot.commands[0U];
    if (record.action.value != ACTION_B.value) {
        return Fail("xinput style axis action mismatch");
    }

    if (record.value_kind != InputCommandValueKind::Axis) {
        return Fail("xinput style axis value kind mismatch");
    }

    if (record.axis_value != AXIS_NEGATIVE) {
        return Fail("xinput style axis value mismatch");
    }

    if (record.pressed_this_frame || record.released_this_frame || record.held) {
        return Fail("xinput style axis emitted button state");
    }

    return 0;
}

int InputCommandMapperKeyboardAndXInputFixturesStayRuntimeOnly() {
    ScopedNativeGamepadPollFunction poll_function(PollNativeUnavailable);
    InputBridge bridge;
    InputBridgeDesc desc{};
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("runtime boundary bridge initialize failed");
    }

    if (bridge.PollGamepad(0U) != InputStatus::DeviceUnavailable) {
        return Fail("runtime boundary unavailable xinput did not return explicit status");
    }

    if (bridge.Snapshot().gamepad_connection != InputGamepadConnection::Unavailable) {
        return Fail("runtime boundary unavailable xinput connection mismatch");
    }

    InputCommandMapper mapper;
    if (!RegisterCommandContext(mapper)) {
        return Fail("runtime boundary command context registration failed");
    }

    const InputControlId gamepad_axis{GAMEPAD_LEFT_THUMB_X_CONTROL};
    if (mapper.RegisterBinding(ButtonCommandBinding(CONTEXT_A, DEVICE_A, CONTROL_A, ACTION_A)) != InputStatus::Success) {
        return Fail("runtime boundary keyboard binding failed");
    }

    if (mapper.RegisterBinding(AxisCommandBinding(CONTEXT_A, DEVICE_B, gamepad_axis, ACTION_B)) != InputStatus::Success) {
        return Fail("runtime boundary xinput axis binding failed");
    }

    std::array<InputEvent, 2U> events{
        ButtonPress(DEVICE_A, CONTROL_A),
        Axis(DEVICE_B, gamepad_axis, AXIS_NEGATIVE)};
    InputCommandSnapshot snapshot{};
    const InputStatus status = mapper.BuildSnapshot(
        99U,
        std::span<const InputEvent>(events.data(), events.size()),
        &snapshot);
    if (status != InputStatus::Success) {
        return Fail("runtime boundary command snapshot failed");
    }

    if (snapshot.frame_index != 99U) {
        return Fail("runtime boundary command frame mismatch");
    }

    if (snapshot.command_count != 2U) {
        return Fail("runtime boundary command count mismatch");
    }

    const auto &keyboard_record = snapshot.commands[0U];
    if (keyboard_record.action.value != ACTION_A.value) {
        return Fail("runtime boundary keyboard action mismatch");
    }

    if (!keyboard_record.pressed_this_frame || !keyboard_record.held || keyboard_record.released_this_frame) {
        return Fail("runtime boundary keyboard command state mismatch");
    }

    const auto &gamepad_record = snapshot.commands[1U];
    if (gamepad_record.action.value != ACTION_B.value) {
        return Fail("runtime boundary xinput action mismatch");
    }

    if (gamepad_record.value_kind != InputCommandValueKind::Axis) {
        return Fail("runtime boundary xinput value kind mismatch");
    }

    if (gamepad_record.axis_value != AXIS_NEGATIVE) {
        return Fail("runtime boundary xinput axis mismatch");
    }

    if (gamepad_record.pressed_this_frame || gamepad_record.released_this_frame || gamepad_record.held) {
        return Fail("runtime boundary xinput emitted button state");
    }

    return 0;
}

int InputCommandMapperFocusRejectsInputWithoutMutation() {
    InputCommandMapper mapper;
    if (!RegisterCommandContext(mapper)) {
        return Fail("command context registration failed");
    }

    if (mapper.RegisterBinding(ButtonCommandBinding(CONTEXT_A, DEVICE_A, CONTROL_A, ACTION_A)) != InputStatus::Success) {
        return Fail("keyboard command binding failed");
    }

    if (mapper.SetActiveContext(CONTEXT_A, InputContextFocusMode::RejectInput) != InputStatus::Success) {
        return Fail("focus reject setup failed");
    }

    std::array<InputEvent, 1U> press_events{ButtonPress(DEVICE_A, CONTROL_A)};
    InputCommandSnapshot snapshot{};
    const InputStatus status = mapper.BuildSnapshot(
        9U,
        std::span<const InputEvent>(press_events.data(), press_events.size()),
        &snapshot);
    if (status != InputStatus::FocusLost) {
        return Fail("focus reject did not return focus lost");
    }

    if (snapshot.command_count != 0U) {
        return Fail("focus reject emitted commands");
    }

    if (mapper.Snapshot().build_count != 0U) {
        return Fail("focus reject mutated build count");
    }

    return 0;
}

int InputCommandMapperInvalidEventDoesNotMutateHeldState() {
    InputCommandMapper mapper;
    if (!RegisterCommandContext(mapper)) {
        return Fail("command context registration failed");
    }

    if (mapper.RegisterBinding(ButtonCommandBinding(CONTEXT_A, DEVICE_A, CONTROL_A, ACTION_A)) != InputStatus::Success) {
        return Fail("keyboard command binding failed");
    }

    std::array<InputEvent, 1U> press_events{ButtonPress(DEVICE_A, CONTROL_A)};
    InputCommandSnapshot press_snapshot{};
    if (mapper.BuildSnapshot(
        1U,
        std::span<const InputEvent>(press_events.data(), press_events.size()),
        &press_snapshot) != InputStatus::Success) {
        return Fail("initial press snapshot failed");
    }

    std::array<InputEvent, 1U> invalid_events{Axis(DEVICE_A, CONTROL_A, AXIS_POSITIVE)};
    InputCommandSnapshot invalid_snapshot{};
    if (mapper.BuildSnapshot(
        2U,
        std::span<const InputEvent>(invalid_events.data(), invalid_events.size()),
        &invalid_snapshot) != InputStatus::InvalidEvent) {
        return Fail("invalid button axis did not return explicit status");
    }

    InputCommandSnapshot held_snapshot{};
    if (mapper.BuildSnapshot(3U, std::span<const InputEvent>(), &held_snapshot) != InputStatus::Success) {
        return Fail("post-invalid held snapshot failed");
    }

    if (held_snapshot.command_count != 1U) {
        return Fail("post-invalid held command count mismatch");
    }

    const auto &record = held_snapshot.commands[0U];
    if (!record.held || record.pressed_this_frame || record.released_this_frame) {
        return Fail("invalid event mutated held command state");
    }

    return 0;
}

int InputCommandMapperSetupFailuresDoNotCountRejectedEvents() {
    InputCommandMapper mapper;
    if (mapper.RegisterContext(UNKNOWN_CONTEXT) != InputStatus::UnknownContext) {
        return Fail("invalid command context did not return explicit status");
    }

    auto mapper_snapshot = mapper.Snapshot();
    if (mapper_snapshot.failed_operation_count != 1U) {
        return Fail("invalid context registration did not count failed operation");
    }

    if (mapper_snapshot.rejected_event_count != 0U) {
        return Fail("invalid context registration counted rejected event");
    }

    if (mapper_snapshot.last_status != InputStatus::UnknownContext) {
        return Fail("invalid context registration did not update last status");
    }

    if (mapper.RegisterBinding(ButtonCommandBinding(CONTEXT_A, DEVICE_A, CONTROL_A, ACTION_A)) != InputStatus::UnknownContext) {
        return Fail("binding without context did not return explicit status");
    }

    mapper_snapshot = mapper.Snapshot();
    if (mapper_snapshot.failed_operation_count != 2U) {
        return Fail("binding setup failure did not count failed operation");
    }

    if (mapper_snapshot.rejected_event_count != 0U) {
        return Fail("binding setup failure counted rejected event");
    }

    if (mapper.RegisterContext(CONTEXT_A) != InputStatus::Success) {
        return Fail("command context registration failed");
    }

    if (mapper.SetActiveContext(CONTEXT_B, InputContextFocusMode::AcceptInput) != InputStatus::UnknownContext) {
        return Fail("unknown active context did not return explicit status");
    }

    mapper_snapshot = mapper.Snapshot();
    if (mapper_snapshot.failed_operation_count != 3U) {
        return Fail("active context setup failure did not count failed operation");
    }

    if (mapper_snapshot.rejected_event_count != 0U) {
        return Fail("active context setup failure counted rejected event");
    }

    if (mapper.SetActiveContext(CONTEXT_A, InputContextFocusMode::AcceptInput) != InputStatus::Success) {
        return Fail("active context setup failed");
    }

    if (mapper.RegisterBinding(ButtonCommandBinding(CONTEXT_A, DEVICE_A, CONTROL_A, ACTION_A)) != InputStatus::Success) {
        return Fail("command binding failed");
    }

    std::array<InputEvent, 1U> invalid_events{Axis(DEVICE_A, CONTROL_A, AXIS_POSITIVE)};
    InputCommandSnapshot command_snapshot{};
    if (mapper.BuildSnapshot(
            5U,
            std::span<const InputEvent>(invalid_events.data(), invalid_events.size()),
            &command_snapshot) != InputStatus::InvalidEvent) {
        return Fail("invalid command event did not return explicit status");
    }

    if (command_snapshot.status != InputStatus::InvalidEvent) {
        return Fail("invalid command event did not set output status");
    }

    mapper_snapshot = mapper.Snapshot();
    if (mapper_snapshot.failed_operation_count != 4U) {
        return Fail("invalid command event did not count failed operation");
    }

    if (mapper_snapshot.rejected_event_count != 1U) {
        return Fail("invalid command event did not count rejected event");
    }

    if (mapper_snapshot.last_status != InputStatus::InvalidEvent) {
        return Fail("invalid command event did not update last status");
    }

    return 0;
}

int InputCommandMapperCapacityFailuresRecordEntryIdentity() {
    InputCommandMapper mapper;
    if (!RegisterCommandContext(mapper)) {
        return Fail("command mapper setup failed");
    }

    for (std::size_t index = 0U; index < MAX_INPUT_BINDINGS; ++index) {
        const std::uint32_t control_value = static_cast<std::uint32_t>(100U + index);
        const std::uint32_t action_value = static_cast<std::uint32_t>(index % 2U);
        const InputControlId control{control_value};
        const InputActionId action{action_value};
        const InputCommandBinding binding = ButtonCommandBinding(CONTEXT_A, DEVICE_A, control, action);
        if (mapper.RegisterBinding(binding) != InputStatus::Success) {
            return Fail("command binding capacity fixture setup failed");
        }
    }

    const InputControlId overflow_control{9999U};
    const InputCommandBinding overflow_binding =
        ButtonCommandBinding(CONTEXT_A, DEVICE_B, overflow_control, ACTION_B);
    if (mapper.RegisterBinding(overflow_binding) != InputStatus::CapacityExceeded) {
        return Fail("command binding capacity overflow did not return explicit status");
    }

    auto mapper_snapshot = mapper.Snapshot();
    if (mapper_snapshot.last_failed_capacity_context_id.value != CONTEXT_A.value ||
        mapper_snapshot.last_failed_capacity_action_id.value != ACTION_B.value ||
        mapper_snapshot.last_failed_capacity_device_id.value != DEVICE_B.value ||
        mapper_snapshot.last_failed_capacity_control_id.value != overflow_control.value) {
        return Fail("command binding capacity entry identity mismatch");
    }

    if (mapper_snapshot.last_failed_binding_capacity != MAX_INPUT_BINDINGS ||
        mapper_snapshot.last_failed_binding_count != MAX_INPUT_BINDINGS ||
        mapper_snapshot.last_required_binding_count != MAX_INPUT_BINDINGS + 1U) {
        return Fail("command binding capacity entry counts mismatch");
    }

    if (mapper_snapshot.binding_count != MAX_INPUT_BINDINGS ||
        mapper_snapshot.last_status != InputStatus::CapacityExceeded) {
        return Fail("command binding capacity overflow mutated mapper state");
    }

    InputCommandBinding invalid_binding = overflow_binding;
    invalid_binding.context = UNKNOWN_CONTEXT;
    if (mapper.RegisterBinding(invalid_binding) != InputStatus::UnknownContext) {
        return Fail("command binding validation failure did not return explicit status");
    }

    mapper_snapshot = mapper.Snapshot();
    if (!CommandMapperCapacityEntryIsClear(mapper_snapshot)) {
        return Fail("command binding validation failure did not clear capacity entry");
    }

    if (mapper.RegisterBinding(overflow_binding) != InputStatus::CapacityExceeded) {
        return Fail("second command binding overflow did not return explicit status");
    }

    if (mapper.SetActiveContext(CONTEXT_A, InputContextFocusMode::AcceptInput) != InputStatus::Success) {
        return Fail("command mapper success status setup failed");
    }

    mapper_snapshot = mapper.Snapshot();
    if (!CommandMapperCapacityEntryIsClear(mapper_snapshot)) {
        return Fail("command mapper success status did not clear capacity entry");
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
        {TEST_BINDING_CAPACITY_ENTRY, InputBindingCapacityOverflowReportsRejectedEntry},
        {TEST_PRESS_RELEASE, InputReplayFrameAppliesButtonPressAndRelease},
        {TEST_EVENT_ORDER, InputReplayFrameEventOrderIsDeterministic},
        {TEST_LAST_VALID_WINS, InputReplayFrameEventOrderLastValidValueWins},
        {TEST_PRESS_RELEASE_CHANGED, InputReplayFramePressReleaseSameFrameSetsChangedFlag},
        {TEST_AXIS, InputReplayFrameAppliesFixedPointAxisValue},
        {TEST_INVALID_AXIS, InputInvalidAxisValueReturnsExplicitStatusWithoutMutation},
        {TEST_INVALID_EVENT, InputInvalidEventDoesNotMutateReplayOrSnapshot},
        {TEST_UNKNOWN_IDS, InputUnknownDeviceControlOrActionReturnsExplicitStatus},
        {TEST_EVENT_CAPACITY, InputEventCapacityOverflowDoesNotMutateReplay},
        {TEST_REPLAY_CAPACITY_ENTRY, InputReplayEventCapacityFailureReportsEntry},
        {TEST_SNAPSHOT_DETERMINISM, InputFrameSnapshotIsDeterministicAcrossReplay},
        {TEST_RESET, InputResetClearsChangedStateWithoutClearingPressedState},
        {TEST_DISABLED_DIAGNOSTICS, InputDisabledDiagnosticsDoesNotChangeResults},
        {TEST_NO_GROW, InputFrameApplyDoesNotGrowReplayStorage},
        {TEST_NO_FORBIDDEN_DEPENDENCY, InputNoPlatformUiOrGameAdapterDependency},
        {TEST_BRIDGE_DESC, InputBridgeDescDefaultValuesAreBounded},
        {TEST_BRIDGE_CONTRACT, InputBridgePublicContractUsesValueTypes},
        {TEST_BRIDGE_UNSUPPORTED, InputBridgeInitializeRejectsUnsupportedBackend},
        {TEST_BRIDGE_DRAIN, InputBridgeSubmitAndDrainRecordsKeyboardMouseWheel},
        {TEST_BRIDGE_FOCUS, InputBridgeFocusLostRejectsInputAndTracksCounters},
        {TEST_BRIDGE_CAPACITY, InputBridgeCapacityOverflowDoesNotGrow},
        {TEST_BRIDGE_SMALL_DRAIN, InputBridgeDrainRejectsSmallOutputWithoutMutation},
        {TEST_BRIDGE_GAMEPAD_UNAVAILABLE, InputBridgeGamepadUnavailableStateIsExplicit},
        {TEST_BRIDGE_GAMEPAD_CONNECTED, InputBridgeGamepadConnectedStateQueuesButtonAndAxisEvents},
        {TEST_BRIDGE_GAMEPAD_REPEAT, InputBridgeGamepadRepeatedPacketDoesNotQueueDuplicateEvents},
        {TEST_BRIDGE_GAMEPAD_CAPACITY, InputBridgeGamepadCapacityOverflowDoesNotQueuePartialEvents},
        {TEST_BRIDGE_XINPUT_GAMEPAD_CAPACITY, InputBridgeXInputPollGamepadCapacityOverflowRecordsRejectedState},
        {TEST_BRIDGE_XINPUT_UNAVAILABLE, InputBridgeXInputPollUnavailableReturnsDeviceUnavailable},
        {TEST_BRIDGE_XINPUT_CONNECTED, InputBridgeXInputPollConnectedQueuesButtonAxisAndPacket},
        {TEST_BRIDGE_XINPUT_BACKEND_ERROR, InputBridgeXInputPollBackendErrorReturnsExplicitStatus},
        {TEST_BRIDGE_XINPUT_INVALID_USER, InputBridgeXInputPollInvalidUserIndexReturnsExplicitStatus},
        {TEST_BRIDGE_NO_DISPATCH, InputBridgeNoUiGameOrReportDispatch},
        {TEST_COMMAND_KEYBOARD, InputCommandMapperKeyboardBuildsFrameCommandSnapshot},
        {TEST_COMMAND_XINPUT_AXIS, InputCommandMapperXInputStyleAxisBuildsCommandSnapshot},
        {TEST_COMMAND_RUNTIME_BOUNDARY, InputCommandMapperKeyboardAndXInputFixturesStayRuntimeOnly},
        {TEST_COMMAND_FOCUS, InputCommandMapperFocusRejectsInputWithoutMutation},
        {TEST_COMMAND_INVALID, InputCommandMapperInvalidEventDoesNotMutateHeldState},
        {TEST_COMMAND_SETUP_REJECTION, InputCommandMapperSetupFailuresDoNotCountRejectedEvents},
        {TEST_COMMAND_CAPACITY_ENTRY, InputCommandMapperCapacityFailuresRecordEntryIdentity}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
