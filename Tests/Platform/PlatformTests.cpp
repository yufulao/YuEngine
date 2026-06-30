// 模块：Tests Platform
// 文件：Tests/Platform/PlatformTests.cpp

#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "YuEngine/Diagnostics/BoundedInMemoryLogSink.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Platform/FixedFrameClock.h"
#include "YuEngine/Platform/HeadlessHost.h"
#include "YuEngine/Platform/HostStatus.h"
#include "YuEngine/Platform/IHostRuntime.h"
#include "YuEngine/Platform/PlatformNativeSurface.h"
#include "YuEngine/Platform/PlatformPerformanceSignal.h"
#include "YuEngine/Platform/PlatformWindowDesc.h"
#include "YuEngine/Platform/PlatformWindowEvent.h"
#include "YuEngine/Platform/PlatformWindowPollResult.h"
#include "YuEngine/Platform/PlatformWindowSnapshot.h"
#include "YuEngine/Platform/PlatformWindowStatus.h"
#include "YuEngine/Platform/WindowsPlatformWindow.h"

using FixedFrameClock = yuengine::platform::FixedFrameClock;
using HeadlessHost = yuengine::platform::HeadlessHost;
using yuengine::platform::HeadlessHostConfig;
using yuengine::platform::HostError;
using yuengine::platform::HostStatus;
using IHostRuntime = yuengine::platform::IHostRuntime;
using yuengine::memory::MemoryAccountingStatus;
using yuengine::platform::PlatformNativeSurface;
using yuengine::platform::PlatformPerformanceSignal;
using yuengine::platform::PlatformWindowDesc;
using yuengine::platform::PlatformWindowEvent;
using yuengine::platform::PlatformWindowPollResult;
using yuengine::platform::PlatformWindowSnapshot;
using yuengine::platform::PlatformWindowStatus;
using yuengine::platform::WindowsPlatformWindow;
using BoundedInMemoryLogSink = yuengine::diagnostics::BoundedInMemoryLogSink;

namespace yuengine::platform {
struct WindowsPlatformWindowAccess {
    static void ApplyClientExtent(WindowsPlatformWindow& window, std::uint32_t client_width, std::uint32_t client_height) {
        window.ApplyClientExtent(client_width, client_height);
    }

    static void ApplyFocusState(WindowsPlatformWindow& window, bool focused) {
        window.ApplyFocusState(focused);
    }

    static void ApplyMinimizedState(WindowsPlatformWindow& window, bool minimized) {
        window.ApplyMinimizedState(minimized);
    }

    static void ApplyCloseRequest(WindowsPlatformWindow& window) {
        window.ApplyCloseRequest();
    }

    static PlatformWindowStatus PushPlatformEvent(WindowsPlatformWindow& window, const PlatformWindowEvent& event) {
        return window.PushPlatformEvent(event);
    }
};
}

namespace {
constexpr const char* TEST_HOST = "Host_StartTickShutdown_Deterministic";
constexpr const char* TEST_TIMER = "Host_TimerMonotonic_ForFixedTicks";
constexpr const char* TEST_MEMORY_STATUS = "Platform_AllocationAccountingStatus_UsesMemoryHook";
constexpr const char* TEST_WINDOW_DESC_BOUNDED = "PlatformWindow_DescDefaultValuesAreBounded";
constexpr const char* TEST_WINDOW_INVALID_DESC = "PlatformWindow_InvalidDescriptorRejected";
constexpr const char* TEST_WINDOW_SURFACE_DEFAULT = "PlatformWindow_SurfaceDefaultInvalid";
constexpr const char* TEST_WINDOW_INITIAL_SNAPSHOT = "PlatformWindow_InitialSnapshotIsEmpty";
constexpr const char* TEST_WINDOW_DESTROY_IDEMPOTENT = "PlatformWindow_DestroyIsIdempotent";
constexpr const char* TEST_WINDOW_NULL_POLL = "PlatformWindow_PollEventsRejectsNullOutputBuffer";
constexpr const char* TEST_WINDOW_POLL_NOT_CREATED = "PlatformWindow_PollEventsBeforeCreateReturnsNotCreated";
constexpr const char* TEST_WINDOW_OPS_NOT_CREATED = "PlatformWindow_OperationsBeforeCreateReturnNotCreated";
constexpr const char* TEST_WINDOW_QUEUE_BOUNDED = "PlatformWindow_QueueCapacityLimitIsBounded";
constexpr const char* TEST_WINDOW_EVENT_OVERFLOW_STATUS = "PlatformWindow_EventOverflowReportsSnapshotStatus";
constexpr const char* TEST_WINDOW_PLAIN_TYPES = "PlatformWindow_PublicTypesArePlainValues";
constexpr const char* LOG_MODULE_PLATFORM = "Platform";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint64_t FIRST_TICK_NANOSECONDS = 1000U;
constexpr std::uint64_t STEP_NANOSECONDS = 16U;
constexpr std::uint32_t DETERMINISTIC_TICK_COUNT = 3U;
constexpr std::uint32_t TIMER_TICK_COUNT = 4U;
constexpr std::size_t LOG_CAPACITY = 16U;
using TestFunction = int (*)();

class TraceRuntime final : public IHostRuntime {
public:
    HostError Start(std::vector<std::string>& lifecycle_trace) override {
        lifecycle_trace.push_back("runtime.start");
        return HostError::Success();
    }

    HostError Tick(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) override {
        static_cast<void>(frame_index);
        static_cast<void>(tick_time_nanoseconds);
        lifecycle_trace.push_back("runtime.tick");
        return HostError::Success();
    }

    HostError Shutdown(std::vector<std::string>& lifecycle_trace) override {
        lifecycle_trace.push_back("runtime.shutdown");
        return HostError::Success();
    }
};

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int HostStartTickShutdownDeterministic() {
    BoundedInMemoryLogSink log_sink(LOG_CAPACITY);
    FixedFrameClock frame_clock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime runtime;
    HeadlessHost host(frame_clock, log_sink);

    const HeadlessHostConfig config{DETERMINISTIC_TICK_COUNT};
    const auto result = host.Run(runtime, config);
    if (result.status != HostStatus::Success) {
        return Fail("host did not return success");
    }

    if (result.tick_count != DETERMINISTIC_TICK_COUNT) {
        return Fail("host tick count was not deterministic");
    }

    const std::vector<std::string> expected_trace{
        "host.start",
        "runtime.start",
        "host.tick",
        "runtime.tick",
        "host.tick",
        "runtime.tick",
        "host.tick",
        "runtime.tick",
        "host.shutdown",
        "runtime.shutdown"};

    if (result.lifecycle_trace != expected_trace) {
        return Fail("host lifecycle trace did not match expected order");
    }

    if (result.allocation_accounting_status != PlatformPerformanceSignal::ALLOCATION_ACCOUNTING_STATUS) {
        return Fail("allocation accounting status is missing");
    }

    if (log_sink.Events().empty()) {
        return Fail("recording log sink did not receive host events");
    }

    if (log_sink.Events()[0U].module_name != LOG_MODULE_PLATFORM) {
        return Fail("host log module was not recorded");
    }

    return 0;
}

int HostTimerMonotonicForFixedTicks() {
    BoundedInMemoryLogSink log_sink(LOG_CAPACITY);
    FixedFrameClock frame_clock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime runtime;
    HeadlessHost host(frame_clock, log_sink);

    const HeadlessHostConfig config{TIMER_TICK_COUNT};
    const auto result = host.Run(runtime, config);
    if (result.tick_times_nanoseconds.size() != TIMER_TICK_COUNT) {
        return Fail("timer did not record every tick");
    }

    for (std::size_t index = 1U; index < result.tick_times_nanoseconds.size(); ++index) {
        if (result.tick_times_nanoseconds[index] <= result.tick_times_nanoseconds[index - 1U]) {
            return Fail("timer ticks were not monotonic");
        }
    }

    if (result.tick_times_nanoseconds[0U] != FIRST_TICK_NANOSECONDS) {
        return Fail("first timer tick was unexpected");
    }

    if (result.tick_times_nanoseconds[3U] != FIRST_TICK_NANOSECONDS + (STEP_NANOSECONDS * 3U)) {
        return Fail("fixed timer step was unexpected");
    }

    return 0;
}

int PlatformAllocationAccountingStatusUsesMemoryHook() {
    BoundedInMemoryLogSink log_sink(LOG_CAPACITY);
    FixedFrameClock frame_clock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime runtime;
    HeadlessHost host(frame_clock, log_sink);

    const HeadlessHostConfig config{DETERMINISTIC_TICK_COUNT};
    const auto result = host.Run(runtime, config);
    if (result.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("platform did not expose typed explicit-tracking memory status");
    }

    return 0;
}

int PlatformWindowDescDefaultValuesAreBounded() {
    const PlatformWindowDesc desc{};
    if (desc.title == nullptr) {
        return Fail("platform window title default was null");
    }

    if (desc.client_width < PlatformWindowDesc::MIN_CLIENT_SIZE) {
        return Fail("platform window width default was below minimum");
    }

    if (desc.client_width > PlatformWindowDesc::MAX_CLIENT_SIZE) {
        return Fail("platform window width default was above maximum");
    }

    if (desc.client_height < PlatformWindowDesc::MIN_CLIENT_SIZE) {
        return Fail("platform window height default was below minimum");
    }

    if (desc.client_height > PlatformWindowDesc::MAX_CLIENT_SIZE) {
        return Fail("platform window height default was above maximum");
    }

    if (desc.event_queue_capacity == 0U) {
        return Fail("platform window queue default was empty");
    }

    if (desc.event_queue_capacity > PlatformWindowDesc::MAX_EVENT_QUEUE_CAPACITY) {
        return Fail("platform window queue default exceeded maximum");
    }

    return 0;
}

int PlatformWindowInvalidDescriptorRejected() {
    WindowsPlatformWindow window;
    PlatformWindowDesc desc{};
    desc.client_width = 0U;

    const PlatformWindowStatus status = window.Create(desc);
    if (status != PlatformWindowStatus::InvalidDescriptor) {
        return Fail("invalid platform window descriptor was not rejected");
    }

    if (window.IsCreated()) {
        return Fail("invalid descriptor created a platform window");
    }

    const PlatformNativeSurface surface = window.GetNativeSurface();
    if (surface.valid) {
        return Fail("invalid descriptor produced a native surface");
    }

    const PlatformWindowSnapshot snapshot = window.GetSnapshot();
    if (snapshot.last_status != PlatformWindowStatus::InvalidDescriptor) {
        return Fail("invalid descriptor did not update status");
    }

    return 0;
}

int PlatformWindowSurfaceDefaultInvalid() {
    const PlatformNativeSurface surface{};
    if (surface.valid) {
        return Fail("default native surface was valid");
    }

    if (surface.window_value != 0U) {
        return Fail("default native surface window value was non-zero");
    }

    if (surface.instance_value != 0U) {
        return Fail("default native surface instance value was non-zero");
    }

    return 0;
}

int PlatformWindowInitialSnapshotIsEmpty() {
    WindowsPlatformWindow window;
    const PlatformWindowSnapshot snapshot = window.GetSnapshot();
    if (snapshot.created) {
        return Fail("initial window snapshot was created");
    }

    if (snapshot.native_surface.valid) {
        return Fail("initial window snapshot had a valid native surface");
    }

    if (snapshot.queued_event_count != 0U) {
        return Fail("initial window snapshot had queued events");
    }

    if (snapshot.event_queue_capacity != PlatformWindowDesc::DEFAULT_EVENT_QUEUE_CAPACITY) {
        return Fail("initial window snapshot had unexpected queue capacity");
    }

    if (snapshot.last_status != PlatformWindowStatus::NotCreated) {
        return Fail("initial window snapshot had unexpected status");
    }

    return 0;
}

int PlatformWindowDestroyIsIdempotent() {
    WindowsPlatformWindow window;
    const PlatformWindowStatus first_status = window.Destroy();
    if (first_status != PlatformWindowStatus::Success) {
        return Fail("first platform window destroy failed");
    }

    const PlatformWindowStatus second_status = window.Destroy();
    if (second_status != PlatformWindowStatus::Success) {
        return Fail("second platform window destroy failed");
    }

    const PlatformWindowSnapshot snapshot = window.GetSnapshot();
    if (snapshot.created) {
        return Fail("destroyed platform window remained created");
    }

    if (snapshot.native_surface.valid) {
        return Fail("destroyed platform window kept a native surface");
    }

    if (snapshot.last_status != PlatformWindowStatus::Success) {
        return Fail("destroyed platform window did not keep success status");
    }

    return 0;
}

int PlatformWindowPollEventsRejectsNullOutputBuffer() {
    WindowsPlatformWindow window;
    const PlatformWindowPollResult poll_result = window.PollEvents(nullptr, 1U);
    if (poll_result.status != PlatformWindowStatus::NullPointer) {
        return Fail("platform window poll did not reject null output");
    }

    if (poll_result.event_count != 0U) {
        return Fail("platform window poll wrote events to null output");
    }

    return 0;
}

int PlatformWindowPollEventsBeforeCreateReturnsNotCreated() {
    WindowsPlatformWindow window;
    PlatformWindowEvent events[1U]{};
    const PlatformWindowPollResult poll_result = window.PollEvents(events, 1U);
    if (poll_result.status != PlatformWindowStatus::NotCreated) {
        return Fail("platform window poll before create did not return not-created");
    }

    if (poll_result.event_count != 0U) {
        return Fail("platform window poll before create returned events");
    }

    return 0;
}

int PlatformWindowOperationsBeforeCreateReturnNotCreated() {
    WindowsPlatformWindow window;
    if (window.Show() != PlatformWindowStatus::NotCreated) {
        return Fail("platform window show before create did not return not-created");
    }

    if (window.Hide() != PlatformWindowStatus::NotCreated) {
        return Fail("platform window hide before create did not return not-created");
    }

    if (window.RequestClose() != PlatformWindowStatus::NotCreated) {
        return Fail("platform window request close before create did not return not-created");
    }

    return 0;
}

int PlatformWindowQueueCapacityLimitIsBounded() {
    static_assert(PlatformWindowDesc::DEFAULT_EVENT_QUEUE_CAPACITY > 0U);
    static_assert(PlatformWindowDesc::DEFAULT_EVENT_QUEUE_CAPACITY <= PlatformWindowDesc::MAX_EVENT_QUEUE_CAPACITY);

    PlatformWindowDesc desc{};
    desc.event_queue_capacity = PlatformWindowDesc::MAX_EVENT_QUEUE_CAPACITY + 1U;
    WindowsPlatformWindow window;
    const PlatformWindowStatus status = window.Create(desc);
    if (status != PlatformWindowStatus::InvalidDescriptor) {
        return Fail("oversized platform window queue was not rejected");
    }

    return 0;
}

int PlatformWindowEventOverflowReportsSnapshotStatus() {
    WindowsPlatformWindow window;
    for (std::size_t index = 0U; index < PlatformWindowDesc::DEFAULT_EVENT_QUEUE_CAPACITY; ++index) {
        PlatformWindowEvent queued_event{};
        queued_event.type = yuengine::platform::PlatformWindowEventType::RawKeyDown;
        queued_event.raw_code = static_cast<std::uint32_t>(index);
        const PlatformWindowStatus status = yuengine::platform::WindowsPlatformWindowAccess::PushPlatformEvent(window, queued_event);
        if (status != PlatformWindowStatus::Success) {
            return Fail("internal platform event enqueue failed before capacity");
        }
    }

    PlatformWindowEvent second_event{};
    second_event.type = yuengine::platform::PlatformWindowEventType::FocusLost;
    const PlatformWindowStatus second_status = yuengine::platform::WindowsPlatformWindowAccess::PushPlatformEvent(window, second_event);
    if (second_status != PlatformWindowStatus::EventQueueOverflow) {
        return Fail("internal platform event overflow did not return overflow status");
    }

    PlatformWindowSnapshot snapshot = window.GetSnapshot();
    if (snapshot.queued_event_count != PlatformWindowDesc::DEFAULT_EVENT_QUEUE_CAPACITY) {
        return Fail("internal platform event overflow changed queued count");
    }

    if (snapshot.dropped_event_count != 1U) {
        return Fail("internal platform event overflow did not record dropped count");
    }

    if (snapshot.last_status != PlatformWindowStatus::EventQueueOverflow) {
        return Fail("internal platform event overflow did not record snapshot status");
    }

    return 0;
}

int PlatformWindowPublicTypesArePlainValues() {
    static_assert(std::is_trivially_copyable<PlatformNativeSurface>::value);
    static_assert(std::is_trivially_copyable<PlatformWindowEvent>::value);
    static_assert(std::is_trivially_copyable<PlatformWindowPollResult>::value);
    static_assert(std::is_trivially_copyable<PlatformWindowSnapshot>::value);

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_HOST, HostStartTickShutdownDeterministic},
        {TEST_TIMER, HostTimerMonotonicForFixedTicks},
        {TEST_MEMORY_STATUS, PlatformAllocationAccountingStatusUsesMemoryHook},
        {TEST_WINDOW_DESC_BOUNDED, PlatformWindowDescDefaultValuesAreBounded},
        {TEST_WINDOW_INVALID_DESC, PlatformWindowInvalidDescriptorRejected},
        {TEST_WINDOW_SURFACE_DEFAULT, PlatformWindowSurfaceDefaultInvalid},
        {TEST_WINDOW_INITIAL_SNAPSHOT, PlatformWindowInitialSnapshotIsEmpty},
        {TEST_WINDOW_DESTROY_IDEMPOTENT, PlatformWindowDestroyIsIdempotent},
        {TEST_WINDOW_NULL_POLL, PlatformWindowPollEventsRejectsNullOutputBuffer},
        {TEST_WINDOW_POLL_NOT_CREATED, PlatformWindowPollEventsBeforeCreateReturnsNotCreated},
        {TEST_WINDOW_OPS_NOT_CREATED, PlatformWindowOperationsBeforeCreateReturnNotCreated},
        {TEST_WINDOW_QUEUE_BOUNDED, PlatformWindowQueueCapacityLimitIsBounded},
        {TEST_WINDOW_EVENT_OVERFLOW_STATUS, PlatformWindowEventOverflowReportsSnapshotStatus},
        {TEST_WINDOW_PLAIN_TYPES, PlatformWindowPublicTypesArePlainValues}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
