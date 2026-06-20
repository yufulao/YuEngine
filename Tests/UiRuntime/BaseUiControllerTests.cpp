// 模块: Tests UiRuntime
// 文件: Tests/UiRuntime/BaseUiControllerTests.cpp

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiRuntime/BaseUiController.h"
#include "YuEngine/UiRuntime/BaseUiLifecycleSnapshot.h"
#include "YuEngine/UiRuntime/BaseUiLifecycleState.h"
#include "YuEngine/UiRuntime/BaseUiLifecycleStatus.h"
#include "YuEngine/UiRuntime/IBaseUiCloseRequestSink.h"

using yuengine::uiruntime::BaseUiController;
using yuengine::uiruntime::BaseUiLifecycleSnapshot;
using yuengine::uiruntime::BaseUiLifecycleState;
using yuengine::uiruntime::BaseUiLifecycleStatus;
using yuengine::uiruntime::IBaseUiCloseRequestSink;

namespace {
constexpr const char *TEST_INITIALIZE_BINDS_ONCE =
    "UiRuntime_BaseUiController_InitializeBindsOnce";
constexpr const char *TEST_OPEN_CLOSE_PER_ACTIVATION =
    "UiRuntime_BaseUiController_OpenClosePerActivation";
constexpr const char *TEST_DESTROY_CLEARS_ONCE =
    "UiRuntime_BaseUiController_DestroyClearsOnce";
constexpr const char *TEST_CLOSE_SELF_THROUGH_MANAGER =
    "UiRuntime_BaseUiController_CloseSelfThroughManager";
constexpr const char *TEST_REJECTS_INVALID_CALLS =
    "UiRuntime_BaseUiController_RejectsInvalidLifecycleCalls";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::size_t MAX_TEST_EVENT_COUNT = 16U;

enum class TestLifecycleEvent {
    None,
    Init,
    Bind,
    Open,
    Close,
    Clear
};

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

class TestBaseUiController final : public BaseUiController {
public:
    std::size_t EventCount() const {
        return event_count_;
    }

    TestLifecycleEvent EventAt(std::size_t index) const {
        if (index >= event_count_) {
            return TestLifecycleEvent::None;
        }

        return events_[index];
    }

protected:
    BaseUiLifecycleStatus OnInitEvent() override {
        return PushEvent(TestLifecycleEvent::Init);
    }

    BaseUiLifecycleStatus OnBindEvent() override {
        return PushEvent(TestLifecycleEvent::Bind);
    }

    BaseUiLifecycleStatus OnOpenEvent() override {
        return PushEvent(TestLifecycleEvent::Open);
    }

    BaseUiLifecycleStatus OnCloseEvent() override {
        return PushEvent(TestLifecycleEvent::Close);
    }

    BaseUiLifecycleStatus OnClearEvent() override {
        return PushEvent(TestLifecycleEvent::Clear);
    }

private:
    BaseUiLifecycleStatus PushEvent(TestLifecycleEvent event) {
        if (event_count_ >= events_.size()) {
            return BaseUiLifecycleStatus::HookFailed;
        }

        events_[event_count_] = event;
        ++event_count_;
        return BaseUiLifecycleStatus::Success;
    }

    std::array<TestLifecycleEvent, MAX_TEST_EVENT_COUNT> events_{};
    std::size_t event_count_ = 0U;
};

class TestCloseRequestSink final : public IBaseUiCloseRequestSink {
public:
    BaseUiLifecycleStatus RequestCloseSelf(BaseUiController *controller) override {
        if (controller == nullptr) {
            return BaseUiLifecycleStatus::InvalidController;
        }

        ++request_count_;
        return controller->Close();
    }

    std::uint32_t RequestCount() const {
        return request_count_;
    }

private:
    std::uint32_t request_count_ = 0U;
};

int RequireStatus(BaseUiLifecycleStatus actual, BaseUiLifecycleStatus expected, std::string_view message) {
    if (actual == expected) {
        return 0;
    }

    return Fail(message);
}

int RequireEvent(
    const TestBaseUiController &controller,
    std::size_t index,
    TestLifecycleEvent expected,
    std::string_view message) {
    if (controller.EventAt(index) == expected) {
        return 0;
    }

    return Fail(message);
}

int VerifyInitBindSnapshot(const BaseUiLifecycleSnapshot &snapshot) {
    if (!snapshot.initialized || !snapshot.events_bound) {
        return Fail("controller did not initialize and bind events");
    }

    if (snapshot.initialize_count != 1U || snapshot.bind_event_count != 1U) {
        return Fail("init or bind event count mismatch");
    }

    if (snapshot.open_count != 0U || snapshot.close_count != 0U || snapshot.clear_count != 0U) {
        return Fail("init path touched activation or clear counters");
    }

    if (snapshot.state != BaseUiLifecycleState::Initialized) {
        return Fail("init state mismatch");
    }

    return 0;
}

int RunInitializeBindsOnceTest() {
    TestBaseUiController controller;
    if (RequireStatus(controller.Initialize(), BaseUiLifecycleStatus::Success, "first init failed") != 0) {
        return 1;
    }

    if (RequireStatus(controller.Initialize(), BaseUiLifecycleStatus::Success, "second init failed") != 0) {
        return 1;
    }

    const BaseUiLifecycleSnapshot snapshot = controller.Snapshot();
    if (VerifyInitBindSnapshot(snapshot) != 0) {
        return 1;
    }

    if (controller.EventCount() != 2U) {
        return Fail("initialize path did not run exactly two hooks");
    }

    if (RequireEvent(controller, 0U, TestLifecycleEvent::Init, "init event order mismatch") != 0) {
        return 1;
    }

    return RequireEvent(controller, 1U, TestLifecycleEvent::Bind, "bind event order mismatch");
}

int RunOpenClosePerActivationTest() {
    TestBaseUiController controller;
    if (RequireStatus(controller.Open(), BaseUiLifecycleStatus::Success, "first open failed") != 0) {
        return 1;
    }

    if (RequireStatus(controller.Close(), BaseUiLifecycleStatus::Success, "first close failed") != 0) {
        return 1;
    }

    if (RequireStatus(controller.Open(), BaseUiLifecycleStatus::Success, "second open failed") != 0) {
        return 1;
    }

    if (RequireStatus(controller.Close(), BaseUiLifecycleStatus::Success, "second close failed") != 0) {
        return 1;
    }

    const BaseUiLifecycleSnapshot snapshot = controller.Snapshot();
    if (snapshot.initialize_count != 1U || snapshot.bind_event_count != 1U) {
        return Fail("open path did not initialize once");
    }

    if (snapshot.open_count != 2U || snapshot.close_count != 2U) {
        return Fail("open close activation count mismatch");
    }

    if (snapshot.state != BaseUiLifecycleState::Closed || snapshot.open) {
        return Fail("open close final state mismatch");
    }

    if (RequireEvent(controller, 0U, TestLifecycleEvent::Init, "event 0 mismatch") != 0) {
        return 1;
    }

    if (RequireEvent(controller, 1U, TestLifecycleEvent::Bind, "event 1 mismatch") != 0) {
        return 1;
    }

    if (RequireEvent(controller, 2U, TestLifecycleEvent::Open, "event 2 mismatch") != 0) {
        return 1;
    }

    if (RequireEvent(controller, 3U, TestLifecycleEvent::Close, "event 3 mismatch") != 0) {
        return 1;
    }

    if (RequireEvent(controller, 4U, TestLifecycleEvent::Open, "event 4 mismatch") != 0) {
        return 1;
    }

    return RequireEvent(controller, 5U, TestLifecycleEvent::Close, "event 5 mismatch");
}

int RunDestroyClearsOnceTest() {
    TestBaseUiController controller;
    if (RequireStatus(controller.Open(), BaseUiLifecycleStatus::Success, "open before destroy failed") != 0) {
        return 1;
    }

    if (RequireStatus(controller.Destroy(), BaseUiLifecycleStatus::Success, "destroy failed") != 0) {
        return 1;
    }

    if (RequireStatus(controller.Destroy(), BaseUiLifecycleStatus::AlreadyDestroyed, "second destroy status mismatch") != 0) {
        return 1;
    }

    const BaseUiLifecycleSnapshot snapshot = controller.Snapshot();
    if (!snapshot.destroyed || snapshot.open) {
        return Fail("destroy final flags mismatch");
    }

    if (snapshot.open_count != 1U || snapshot.close_count != 1U || snapshot.clear_count != 1U) {
        return Fail("destroy lifecycle count mismatch");
    }

    if (snapshot.state != BaseUiLifecycleState::Destroyed) {
        return Fail("destroy state mismatch");
    }

    return RequireEvent(controller, 4U, TestLifecycleEvent::Clear, "clear event order mismatch");
}

int RunCloseSelfThroughManagerTest() {
    TestBaseUiController controller;
    TestCloseRequestSink sink;
    if (RequireStatus(controller.Open(), BaseUiLifecycleStatus::Success, "open before self close failed") != 0) {
        return 1;
    }

    if (RequireStatus(controller.RequestCloseSelf(&sink), BaseUiLifecycleStatus::Success, "self close failed") != 0) {
        return 1;
    }

    const BaseUiLifecycleSnapshot snapshot = controller.Snapshot();
    if (sink.RequestCount() != 1U || snapshot.close_self_request_count != 1U) {
        return Fail("manager close request count mismatch");
    }

    if (snapshot.open || snapshot.close_count != 1U) {
        return Fail("manager did not close controller");
    }

    return RequireEvent(controller, 3U, TestLifecycleEvent::Close, "manager close event mismatch");
}

int RunRejectsInvalidLifecycleCallsTest() {
    TestBaseUiController controller;
    if (RequireStatus(controller.Close(), BaseUiLifecycleStatus::NotOpen, "close before open status mismatch") != 0) {
        return 1;
    }

    if (RequireStatus(
        controller.RequestCloseSelf(nullptr),
        BaseUiLifecycleStatus::InvalidCloseRequestSink,
        "null close sink status mismatch") != 0) {
        return 1;
    }

    if (RequireStatus(controller.Open(), BaseUiLifecycleStatus::Success, "open for invalid call test failed") != 0) {
        return 1;
    }

    if (RequireStatus(controller.Open(), BaseUiLifecycleStatus::AlreadyOpen, "double open status mismatch") != 0) {
        return 1;
    }

    if (RequireStatus(controller.Destroy(), BaseUiLifecycleStatus::Success, "destroy for invalid call test failed") != 0) {
        return 1;
    }

    return RequireStatus(controller.Open(), BaseUiLifecycleStatus::Destroyed, "open after destroy status mismatch");
}

int RunNamedTest(std::string_view test_name) {
    if (test_name == TEST_INITIALIZE_BINDS_ONCE) {
        return RunInitializeBindsOnceTest();
    }

    if (test_name == TEST_OPEN_CLOSE_PER_ACTIVATION) {
        return RunOpenClosePerActivationTest();
    }

    if (test_name == TEST_DESTROY_CLEARS_ONCE) {
        return RunDestroyClearsOnceTest();
    }

    if (test_name == TEST_CLOSE_SELF_THROUGH_MANAGER) {
        return RunCloseSelfThroughManagerTest();
    }

    if (test_name == TEST_REJECTS_INVALID_CALLS) {
        return RunRejectsInvalidLifecycleCallsTest();
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
