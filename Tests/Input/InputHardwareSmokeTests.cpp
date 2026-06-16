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
using yuengine::input::InputStatus;

namespace {
constexpr const char *TEST_SOURCE_MESSAGE = "Input_HardwareBridge_TranslatesSourceMessages";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
using TestFunction = int (*)();

int Fail(const std::string &message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
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
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_SOURCE_MESSAGE, InputHardwareBridgeTranslatesSourceMessages}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
