#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "YuEngine/Diagnostics/BoundedInMemoryLogSink.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Platform/FixedFrameClock.h"
#include "YuEngine/Platform/HeadlessHost.h"
#include "YuEngine/Platform/HostStatus.h"
#include "YuEngine/Platform/IHostRuntime.h"
#include "YuEngine/Platform/PlatformPerformanceSignal.h"

using FixedFrameClock = yuengine::platform::FixedFrameClock;
using HeadlessHost = yuengine::platform::HeadlessHost;
using yuengine::platform::HeadlessHostConfig;
using yuengine::platform::HostError;
using yuengine::platform::HostStatus;
using IHostRuntime = yuengine::platform::IHostRuntime;
using yuengine::memory::MemoryAccountingStatus;
using yuengine::platform::PlatformPerformanceSignal;
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
    HostError Start(std::vector<std::string>& lifecycleTrace) override {
        lifecycleTrace.push_back("runtime.start");
        return HostError::Success();
    }

    HostError Tick(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) override {
        static_cast<void>(frameIndex);
        static_cast<void>(tickTimeNanoseconds);
        lifecycleTrace.push_back("runtime.tick");
        return HostError::Success();
    }

    HostError Shutdown(std::vector<std::string>& lifecycleTrace) override {
        lifecycleTrace.push_back("runtime.shutdown");
        return HostError::Success();
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

    const HeadlessHostConfig config{DETERMINISTIC_TICK_COUNT};
    const auto result = host.Run(runtime, config);
    if (result.status != HostStatus::Success) {
        return Fail("host did not return success");
    }

    if (result.tick_count != DETERMINISTIC_TICK_COUNT) {
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

    if (result.lifecycle_trace != expectedTrace) {
        return Fail("host lifecycle trace did not match expected order");
    }

    if (result.allocation_accounting_status != PlatformPerformanceSignal::AllocationAccountingStatus) {
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
    BoundedInMemoryLogSink logSink(LOG_CAPACITY);
    FixedFrameClock frameClock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime runtime;
    HeadlessHost host(frameClock, logSink);

    const HeadlessHostConfig config{DETERMINISTIC_TICK_COUNT};
    const auto result = host.Run(runtime, config);
    if (result.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
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
