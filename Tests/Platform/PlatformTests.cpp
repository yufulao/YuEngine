// Module: Tests Platform
// File: Tests/Platform/PlatformTests.cpp

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
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_HOST, HostStartTickShutdownDeterministic},
        {TEST_TIMER, HostTimerMonotonicForFixedTicks},
        {TEST_MEMORY_STATUS, PlatformAllocationAccountingStatusUsesMemoryHook}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
