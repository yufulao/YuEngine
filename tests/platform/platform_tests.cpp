#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "yuengine/diagnostics/bounded_in_memory_log_sink.h"
#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/platform/fixed_frame_clock.h"
#include "yuengine/platform/headless_host.h"
#include "yuengine/platform/host_status.h"
#include "yuengine/platform/i_host_runtime.h"
#include "yuengine/platform/platform_performance_signal.h"

using FixedFrameClock = yuengine::platform::FixedFrameClock;
using HeadlessHost = yuengine::platform::HeadlessHost;
using yuengine::platform::headless_host_config_t;
using yuengine::platform::host_error_t;
using yuengine::platform::HostStatus;
using IHostRuntime = yuengine::platform::IHostRuntime;
using yuengine::memory::MemoryAccountingStatus;
using yuengine::platform::platform_performance_signal_t;
using BoundedInMemoryLogSink = yuengine::diagnostics::BoundedInMemoryLogSink;

namespace {
constexpr const char* TEST_HOST = "Host_StartTickShutdown_Deterministic";
constexpr const char* TEST_TIMER = "Host_TimerMonotonic_ForFixedTicks";
constexpr const char* TEST_MEMORY_STATUS = "Platform_AllocationAccountingStatus_UsesMemoryHook";
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
    host_error_t Start(std::vector<std::string>& lifecycleTrace) override {
        lifecycleTrace.push_back("runtime.start");
        return host_error_t::Success();
    }

    host_error_t Tick(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) override {
        static_cast<void>(frameIndex);
        static_cast<void>(tickTimeNanoseconds);
        lifecycleTrace.push_back("runtime.tick");
        return host_error_t::Success();
    }

    host_error_t Shutdown(std::vector<std::string>& lifecycleTrace) override {
        lifecycleTrace.push_back("runtime.shutdown");
        return host_error_t::Success();
    }
};

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int HostStartTickShutdownDeterministic() {
    BoundedInMemoryLogSink logSink(LOG_CAPACITY);
    FixedFrameClock frameClock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime runtime;
    HeadlessHost host(frameClock, logSink);

    const headless_host_config_t config{DETERMINISTIC_TICK_COUNT};
    const auto result = host.Run(runtime, config);
    if (result.Status != HostStatus::Success) {
        return Fail("host did not return success");
    }

    if (result.TickCount != DETERMINISTIC_TICK_COUNT) {
        return Fail("host tick count was not deterministic");
    }

    const std::vector<std::string> expectedTrace{
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

    if (result.LifecycleTrace != expectedTrace) {
        return Fail("host lifecycle trace did not match expected order");
    }

    if (result.AllocationAccountingStatus != platform_performance_signal_t::AllocationAccountingStatus) {
        return Fail("allocation accounting status is missing");
    }

    if (logSink.Events().empty()) {
        return Fail("recording log sink did not receive host events");
    }

    return 0;
}

int HostTimerMonotonicForFixedTicks() {
    BoundedInMemoryLogSink logSink(LOG_CAPACITY);
    FixedFrameClock frameClock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime runtime;
    HeadlessHost host(frameClock, logSink);

    const headless_host_config_t config{TIMER_TICK_COUNT};
    const auto result = host.Run(runtime, config);
    if (result.TickTimesNanoseconds.size() != TIMER_TICK_COUNT) {
        return Fail("timer did not record every tick");
    }

    for (std::size_t index = 1U; index < result.TickTimesNanoseconds.size(); ++index) {
        if (result.TickTimesNanoseconds[index] <= result.TickTimesNanoseconds[index - 1U]) {
            return Fail("timer ticks were not monotonic");
        }
    }

    if (result.TickTimesNanoseconds[0U] != FIRST_TICK_NANOSECONDS) {
        return Fail("first timer tick was unexpected");
    }

    if (result.TickTimesNanoseconds[3U] != FIRST_TICK_NANOSECONDS + (STEP_NANOSECONDS * 3U)) {
        return Fail("fixed timer step was unexpected");
    }

    return 0;
}

int PlatformAllocationAccountingStatusUsesMemoryHook() {
    BoundedInMemoryLogSink logSink(LOG_CAPACITY);
    FixedFrameClock frameClock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime runtime;
    HeadlessHost host(frameClock, logSink);

    const headless_host_config_t config{DETERMINISTIC_TICK_COUNT};
    const auto result = host.Run(runtime, config);
    if (result.AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("platform did not expose typed explicit-tracking memory status");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> testRegistry{
        {TEST_HOST, HostStartTickShutdownDeterministic},
        {TEST_TIMER, HostTimerMonotonicForFixedTicks},
        {TEST_MEMORY_STATUS, PlatformAllocationAccountingStatusUsesMemoryHook}};

    const std::string_view testName(argv[1]);
    const auto testIterator = testRegistry.find(testName);
    if (testIterator == testRegistry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return testIterator->second();
}
