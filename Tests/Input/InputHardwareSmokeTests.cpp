// Module: Tests Input
// File: Tests/Input/InputHardwareSmokeTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "YuEngine/Input/InputBridge.h"

using yuengine::input::InputBackendKind;
using yuengine::input::InputBridge;
using yuengine::input::InputBridgeDesc;
using yuengine::input::InputBridgeEvent;
using yuengine::input::InputBridgeEventType;
using yuengine::input::InputDeviceKind;
using yuengine::input::InputGamepadConnection;
using yuengine::input::InputStatus;

namespace {
constexpr const char *TEST_SOURCE_MESSAGE = "Input_HardwareBridge_TranslatesSourceMessages";
constexpr const char *TEST_XINPUT_GAMEPAD = "Input_HardwareBridge_PollsXInputGamepad";
constexpr const char *STRICT_XINPUT_ENVIRONMENT = "YUENGINE_REQUIRE_XINPUT_HARDWARE";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr int SKIP_RETURN_CODE = 77;
using TestFunction = int (*)();

int Fail(const std::string &message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int Skip(const std::string &message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stdout);
    std::fputc('\n', stdout);
    return SKIP_RETURN_CODE;
}

bool IsEnabledEnvironmentValue(const char *value) {
    if (value == nullptr) {
        return false;
    }

    const std::string_view text(value);
    if (text == "1") {
        return true;
    }

    if (text == "true") {
        return true;
    }

    return text == "TRUE";
}

bool IsStrictXInputSmokeRequired() {
    std::array<char, 8U> value{};
    const DWORD value_length = GetEnvironmentVariableA(STRICT_XINPUT_ENVIRONMENT, value.data(), static_cast<DWORD>(value.size()));
    if (value_length == 0U) {
        return false;
    }

    if (value_length >= value.size()) {
        return false;
    }

    return IsEnabledEnvironmentValue(value.data());
}

std::intptr_t MakePointValue(std::int16_t x, std::int16_t y) {
    const std::uint32_t low_word = static_cast<std::uint16_t>(x);
    const std::uint32_t high_word = static_cast<std::uint32_t>(static_cast<std::uint16_t>(y)) << 16U;
    return static_cast<std::intptr_t>(low_word | high_word);
}

int InputHardwareBridgeTranslatesSourceMessages() {
    InputBridgeDesc desc{};
    desc.backend = InputBackendKind::NativeMessage;
    desc.start_focused = false;

    InputBridge bridge;
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    if (bridge.SubmitSourceMessage(WM_SETFOCUS, 0U, 0, true) != InputStatus::Success) {
        return Fail("focus source message failed");
    }

    if (bridge.SubmitSourceMessage(WM_KEYDOWN, static_cast<std::uintptr_t>('A'), 0, true) != InputStatus::Success) {
        return Fail("key source message failed");
    }

    const std::uintptr_t wheel_value = static_cast<std::uintptr_t>(MAKEWPARAM(0U, WHEEL_DELTA));
    if (bridge.SubmitSourceMessage(WM_MOUSEWHEEL, wheel_value, MakePointValue(4, 8), true) != InputStatus::Success) {
        return Fail("wheel source message failed");
    }

    std::array<InputBridgeEvent, 2U> events{};
    std::size_t event_count = 0U;
    if (bridge.DrainEvents(events.data(), events.size(), event_count) != InputStatus::Success) {
        return Fail("source message drain failed");
    }

    if (event_count != events.size()) {
        return Fail("source message event count mismatch");
    }

    if (events[0].type != InputBridgeEventType::KeyPressed) {
        return Fail("key source message did not produce key event");
    }

    if (events[0].raw_code != static_cast<std::uint32_t>('A')) {
        return Fail("key source raw code mismatch");
    }

    if (events[1].type != InputBridgeEventType::MouseWheel) {
        return Fail("wheel source message did not produce wheel event");
    }

    if (events[1].wheel_delta != WHEEL_DELTA) {
        return Fail("wheel source delta mismatch");
    }

    const auto snapshot = bridge.Snapshot();
    if (snapshot.accepted_event_count != 2U) {
        return Fail("accepted source event count mismatch");
    }

    if (snapshot.focus_gained_count != 1U) {
        return Fail("focus source counter mismatch");
    }

    return 0;
}

bool IsGamepadEventType(InputBridgeEventType type) {
    if (type == InputBridgeEventType::GamepadButtonPressed) {
        return true;
    }

    if (type == InputBridgeEventType::GamepadButtonReleased) {
        return true;
    }

    return type == InputBridgeEventType::GamepadAxisMoved;
}

int InputHardwareBridgePollsXInputGamepad() {
    InputBridgeDesc desc{};
    desc.backend = InputBackendKind::NativeMessage;
    desc.event_capacity = InputBridgeDesc::MAX_EVENT_CAPACITY;

    InputBridge bridge;
    if (bridge.Initialize(desc) != InputStatus::Success) {
        return Fail("bridge initialize failed");
    }

    const InputStatus poll_status = bridge.PollGamepad(0U);
    if (poll_status == InputStatus::DeviceUnavailable) {
        if (IsStrictXInputSmokeRequired()) {
            return Fail("xinput gamepad hardware smoke requires a connected controller");
        }

        return Skip("xinput gamepad hardware smoke skipped because no controller is connected");
    }

    if (poll_status != InputStatus::Success) {
        return Fail("xinput gamepad poll failed");
    }

    const auto snapshot = bridge.Snapshot();
    if (snapshot.gamepad_connection != InputGamepadConnection::Connected) {
        return Fail("xinput gamepad connection was not tracked");
    }

    if (snapshot.gamepad_poll_count != 1U) {
        return Fail("xinput gamepad poll count mismatch");
    }

    if (snapshot.gamepad_connected_poll_count != 1U) {
        return Fail("xinput connected poll count mismatch");
    }

    std::array<InputBridgeEvent, InputBridgeDesc::MAX_EVENT_CAPACITY> events{};
    std::size_t event_count = 0U;
    if (bridge.DrainEvents(events.data(), events.size(), event_count) != InputStatus::Success) {
        return Fail("xinput gamepad drain failed");
    }

    for (std::size_t index = 0U; index < event_count; ++index) {
        if (events[index].device_kind != InputDeviceKind::Gamepad) {
            return Fail("xinput gamepad produced non-gamepad event");
        }

        if (!IsGamepadEventType(events[index].type)) {
            return Fail("xinput gamepad produced unexpected event type");
        }
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_SOURCE_MESSAGE, InputHardwareBridgeTranslatesSourceMessages},
        {TEST_XINPUT_GAMEPAD, InputHardwareBridgePollsXInputGamepad}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
