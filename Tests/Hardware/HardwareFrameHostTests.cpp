// 模块：Tests Hardware
// 文件：Tests/Hardware/HardwareFrameHostTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/Hardware/HardwareFrameHost.h"
#include "YuEngine/Hardware/HardwareFrameHostDesc.h"
#include "YuEngine/Hardware/HardwareFrameHostSnapshot.h"
#include "YuEngine/Hardware/HardwareFrameHostStatus.h"
#include "YuEngine/Hardware/HardwareFrameHostTickRequest.h"
#include "YuEngine/Hardware/HardwareFrameHostTickResult.h"
#include "YuEngine/Input/InputBridgeEvent.h"
#include "YuEngine/Input/InputBridgeDesc.h"
#include "YuEngine/Input/InputConstants.h"
#include "YuEngine/Input/InputStatus.h"
#include "YuEngine/Platform/PlatformWindowEvent.h"
#include "YuEngine/Platform/PlatformWindowEventType.h"

using yuengine::hardware::HardwareFrameHost;
using yuengine::hardware::HardwareFrameHostDesc;
using yuengine::hardware::HardwareFrameHostSnapshot;
using yuengine::hardware::HardwareFrameHostStatus;
using yuengine::hardware::HardwareFrameHostTickRequest;
using yuengine::hardware::HardwareFrameHostTickResult;
using yuengine::input::InputBridgeEvent;
using yuengine::input::InputBridgeEventType;
using yuengine::input::InputFocusPolicy;
using yuengine::input::InputStatus;
using yuengine::input::MAX_GAMEPAD_DEVICES;
using yuengine::platform::PlatformWindowEvent;
using yuengine::platform::PlatformWindowEventType;

namespace {
constexpr const char *TEST_TRANSLATES_INPUT = "HardwareFrameHost_TranslatesInjectedPlatformInput";
constexpr const char *TEST_REJECTS_INVALID_DESC = "HardwareFrameHost_RejectsInvalidDescriptor";
constexpr const char *TEST_REJECTS_TICK_BEFORE_INIT = "HardwareFrameHost_RejectsTickBeforeInitialize";
constexpr const char *TEST_GAMEPAD_POLL_INVALID_INDEX = "HardwareFrameHost_GamepadPollRejectsInvalidIndex";
constexpr const char *TEST_SHUTDOWN_IDEMPOTENT = "HardwareFrameHost_ShutdownAfterSuccessIsIdempotent";
constexpr const char *TEST_INVALID_TICK_OUTPUT = "HardwareFrameHost_InvalidTickPreservesCallerOutput";
constexpr const char *TEST_RESIZE_NO_RENDER = "HardwareFrameHost_ResizeEventWithoutRenderCompletesTick";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t TEST_FRAME_ID = 1U;
constexpr std::uint32_t TEST_KEY_CODE = 65U;
constexpr std::int32_t TEST_WHEEL_DELTA = 120;
constexpr std::size_t OUTPUT_COUNT_SENTINEL = 7U;
using TestFunction = int (*)();

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

HardwareFrameHostDesc FastHostDesc() {
    HardwareFrameHostDesc desc{};
    desc.window_desc.title = "YuEngine Hardware Frame Host Fast Test";
    desc.window_desc.client_width = 2U;
    desc.window_desc.client_height = 2U;
    desc.window_desc.visible = false;
    desc.input_desc.start_focused = false;
    desc.input_desc.focus_policy = InputFocusPolicy::RejectWhenUnfocused;
    desc.render_enabled = false;
    desc.audio_enabled = false;
    return desc;
}

PlatformWindowEvent FocusGainedEvent() {
    PlatformWindowEvent event{};
    event.type = PlatformWindowEventType::FocusGained;
    return event;
}

PlatformWindowEvent KeyDownEvent() {
    PlatformWindowEvent event{};
    event.type = PlatformWindowEventType::RawKeyDown;
    event.raw_code = TEST_KEY_CODE;
    return event;
}

PlatformWindowEvent MouseWheelEvent() {
    PlatformWindowEvent event{};
    event.type = PlatformWindowEventType::RawMouseMove;
    event.pointer_x = 4;
    event.pointer_y = 8;
    event.wheel_delta = TEST_WHEEL_DELTA;
    return event;
}

PlatformWindowEvent ResizeEvent() {
    PlatformWindowEvent event{};
    event.type = PlatformWindowEventType::Resized;
    event.client_width = 4U;
    event.client_height = 8U;
    return event;
}

int HardwareFrameHostTranslatesInjectedPlatformInput() {
    HardwareFrameHost host;
    const HardwareFrameHostDesc desc = FastHostDesc();
    const HardwareFrameHostStatus init_status = host.Initialize(desc);
    if (init_status != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host initialize failed");
    }

    std::array<PlatformWindowEvent, 3U> platform_events{};
    platform_events[0U] = FocusGainedEvent();
    platform_events[1U] = KeyDownEvent();
    platform_events[2U] = MouseWheelEvent();

    std::array<InputBridgeEvent, 2U> input_events{};
    std::size_t input_event_count = 0U;
    HardwareFrameHostTickRequest request{};
    request.injected_platform_events = std::span<const PlatformWindowEvent>(
        platform_events.data(),
        platform_events.size());
    request.input_events = std::span<InputBridgeEvent>(input_events.data(), input_events.size());
    request.out_input_event_count = &input_event_count;
    request.frame_id = TEST_FRAME_ID;

    const HardwareFrameHostTickResult result = host.Tick(request);
    if (result.status != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host tick failed");
    }

    if (input_event_count != input_events.size()) {
        return Fail("hardware frame host did not drain expected input count");
    }

    if (input_events[0U].type != InputBridgeEventType::KeyPressed) {
        return Fail("hardware frame host did not translate key press");
    }

    if (input_events[0U].raw_code != TEST_KEY_CODE) {
        return Fail("hardware frame host translated wrong key code");
    }

    if (input_events[1U].type != InputBridgeEventType::MouseWheel) {
        return Fail("hardware frame host did not translate wheel input");
    }

    if (input_events[1U].wheel_delta != TEST_WHEEL_DELTA) {
        return Fail("hardware frame host translated wrong wheel delta");
    }

    const HardwareFrameHostSnapshot snapshot = host.Snapshot();
    if (snapshot.completed_tick_count != 1U) {
        return Fail("hardware frame host did not track completed tick");
    }

    if (snapshot.translated_input_event_count != input_events.size()) {
        return Fail("hardware frame host did not track translated input");
    }

    if (snapshot.drained_input_event_count != input_events.size()) {
        return Fail("hardware frame host did not track drained input");
    }

    if (host.Shutdown() != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host shutdown failed");
    }

    return 0;
}

int HardwareFrameHostRejectsInvalidDescriptor() {
    HardwareFrameHost host;
    HardwareFrameHostDesc desc = FastHostDesc();
    desc.platform_event_capacity = HardwareFrameHostDesc::MAX_PLATFORM_EVENT_CAPACITY + 1U;
    const HardwareFrameHostStatus status = host.Initialize(desc);
    if (status != HardwareFrameHostStatus::InvalidArgument) {
        return Fail("hardware frame host did not reject invalid descriptor");
    }

    return 0;
}

int HardwareFrameHostRejectsTickBeforeInitialize() {
    HardwareFrameHost host;
    std::array<InputBridgeEvent, 1U> input_events{};
    std::size_t input_event_count = 0U;
    HardwareFrameHostTickRequest request{};
    request.input_events = std::span<InputBridgeEvent>(input_events.data(), input_events.size());
    request.out_input_event_count = &input_event_count;
    request.frame_id = TEST_FRAME_ID;

    const HardwareFrameHostTickResult result = host.Tick(request);
    if (result.status != HardwareFrameHostStatus::NotInitialized) {
        return Fail("hardware frame host did not reject tick before initialize");
    }

    return 0;
}

int HardwareFrameHostGamepadPollRejectsInvalidIndex() {
    HardwareFrameHost host;
    const HardwareFrameHostDesc desc = FastHostDesc();
    const HardwareFrameHostStatus init_status = host.Initialize(desc);
    if (init_status != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host initialize failed");
    }

    std::array<InputBridgeEvent, 1U> input_events{};
    std::size_t input_event_count = 0U;
    HardwareFrameHostTickRequest request{};
    request.input_events = std::span<InputBridgeEvent>(input_events.data(), input_events.size());
    request.out_input_event_count = &input_event_count;
    request.frame_id = TEST_FRAME_ID;
    request.poll_gamepad = true;
    request.gamepad_user_index = MAX_GAMEPAD_DEVICES;

    const HardwareFrameHostTickResult result = host.Tick(request);
    if (result.status != HardwareFrameHostStatus::InputPollFailed) {
        return Fail("hardware frame host did not report gamepad poll failure");
    }

    if (!result.gamepad_polled) {
        return Fail("hardware frame host did not mark gamepad poll");
    }

    if (result.gamepad_poll_status != InputStatus::InvalidDescriptor) {
        return Fail("hardware frame host did not report invalid gamepad user index");
    }

    const HardwareFrameHostSnapshot snapshot = host.Snapshot();
    if (snapshot.failed_tick_count != 1U) {
        return Fail("hardware frame host did not track failed gamepad tick");
    }

    if (snapshot.last_input_status != InputStatus::InvalidDescriptor) {
        return Fail("hardware frame host did not store gamepad input status");
    }

    if (host.Shutdown() != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host shutdown failed");
    }

    return 0;
}

int HardwareFrameHostShutdownAfterSuccessIsIdempotent() {
    HardwareFrameHost host;
    const HardwareFrameHostDesc desc = FastHostDesc();
    const HardwareFrameHostStatus init_status = host.Initialize(desc);
    if (init_status != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host initialize failed");
    }

    const HardwareFrameHostStatus first_shutdown_status = host.Shutdown();
    if (first_shutdown_status != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host first shutdown failed");
    }

    const HardwareFrameHostSnapshot first_snapshot = host.Snapshot();
    if (first_snapshot.initialized) {
        return Fail("hardware frame host remained initialized after shutdown");
    }

    const HardwareFrameHostStatus second_shutdown_status = host.Shutdown();
    if (second_shutdown_status != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host second shutdown was not idempotent");
    }

    const HardwareFrameHostSnapshot second_snapshot = host.Snapshot();
    if (second_snapshot.last_status != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host did not preserve shutdown success status");
    }

    if (second_snapshot.initialized) {
        return Fail("hardware frame host re-entered initialized state after shutdown");
    }

    return 0;
}

int HardwareFrameHostInvalidTickPreservesCallerOutput() {
    HardwareFrameHost host;
    const HardwareFrameHostDesc desc = FastHostDesc();
    const HardwareFrameHostStatus init_status = host.Initialize(desc);
    if (init_status != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host initialize failed");
    }

    std::array<InputBridgeEvent, 1U> input_events{};
    std::size_t input_event_count = OUTPUT_COUNT_SENTINEL;
    HardwareFrameHostTickRequest request{};
    request.input_events = std::span<InputBridgeEvent>(input_events.data(), input_events.size());
    request.out_input_event_count = &input_event_count;
    request.frame_id = 0U;

    const HardwareFrameHostTickResult result = host.Tick(request);
    if (result.status != HardwareFrameHostStatus::InvalidArgument) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host did not reject invalid frame id");
    }

    if (input_event_count != OUTPUT_COUNT_SENTINEL) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host mutated caller output on invalid tick");
    }

    const HardwareFrameHostSnapshot snapshot = host.Snapshot();
    if (snapshot.tick_count != 0U) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host counted rejected tick as active tick");
    }

    if (snapshot.failed_tick_count != 1U) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host did not track rejected tick");
    }

    if (host.Shutdown() != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host shutdown failed");
    }

    return 0;
}

int HardwareFrameHostResizeEventWithoutRenderCompletesTick() {
    HardwareFrameHost host;
    const HardwareFrameHostDesc desc = FastHostDesc();
    const HardwareFrameHostStatus init_status = host.Initialize(desc);
    if (init_status != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host initialize failed");
    }

    std::array<PlatformWindowEvent, 1U> platform_events{};
    platform_events[0U] = ResizeEvent();

    std::array<InputBridgeEvent, 1U> input_events{};
    std::size_t input_event_count = OUTPUT_COUNT_SENTINEL;
    HardwareFrameHostTickRequest request{};
    request.injected_platform_events = std::span<const PlatformWindowEvent>(
        platform_events.data(),
        platform_events.size());
    request.input_events = std::span<InputBridgeEvent>(input_events.data(), input_events.size());
    request.out_input_event_count = &input_event_count;
    request.frame_id = TEST_FRAME_ID;

    const HardwareFrameHostTickResult result = host.Tick(request);
    if (result.status != HardwareFrameHostStatus::Success) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host failed resize event tick without render");
    }

    if (input_event_count != 0U) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host emitted input for resize event");
    }

    if (result.translated_input_event_count != 0U) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host translated resize as input");
    }

    const HardwareFrameHostSnapshot snapshot = host.Snapshot();
    if (snapshot.completed_tick_count != 1U) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host did not complete resize tick");
    }

    if (snapshot.platform_event_count == 0U) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host did not track resize event");
    }

    if (host.Shutdown() != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host shutdown failed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_TRANSLATES_INPUT) {
        return HardwareFrameHostTranslatesInjectedPlatformInput();
    }

    if (name == TEST_REJECTS_INVALID_DESC) {
        return HardwareFrameHostRejectsInvalidDescriptor();
    }

    if (name == TEST_REJECTS_TICK_BEFORE_INIT) {
        return HardwareFrameHostRejectsTickBeforeInitialize();
    }

    if (name == TEST_GAMEPAD_POLL_INVALID_INDEX) {
        return HardwareFrameHostGamepadPollRejectsInvalidIndex();
    }

    if (name == TEST_SHUTDOWN_IDEMPOTENT) {
        return HardwareFrameHostShutdownAfterSuccessIsIdempotent();
    }

    if (name == TEST_INVALID_TICK_OUTPUT) {
        return HardwareFrameHostInvalidTickPreservesCallerOutput();
    }

    if (name == TEST_RESIZE_NO_RENDER) {
        return HardwareFrameHostResizeEventWithoutRenderCompletesTick();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
