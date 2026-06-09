#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "yuengine/diagnostics/BoundedInMemoryLogSink.h"
#include "yuengine/platform/FixedFrameClock.h"
#include "yuengine/platform/HeadlessHost.h"
#include "yuengine/platform/HostStatus.h"
#include "yuengine/platform/IHostRuntime.h"
#include "yuengine/platform/PlatformPerformanceSignal.h"

using FixedFrameClock = yuengine::platform::FixedFrameClock;
using HeadlessHost = yuengine::platform::HeadlessHost;
using HeadlessHostConfig = yuengine::platform::HeadlessHostConfig;
using HostError = yuengine::platform::HostError;
using HostStatus = yuengine::platform::HostStatus;
using IHostRuntime = yuengine::platform::IHostRuntime;
using PlatformPerformanceSignal = yuengine::platform::PlatformPerformanceSignal;
using BoundedInMemoryLogSink = yuengine::diagnostics::BoundedInMemoryLogSink;

namespace
{
constexpr const char* TEST_HOST = "Host_StartTickShutdown_Deterministic";
constexpr const char* TEST_TIMER = "Host_TimerMonotonic_ForFixedTicks";
constexpr std::uint64_t FIRST_TICK_NANOSECONDS = 1000U;
constexpr std::uint64_t STEP_NANOSECONDS = 16U;
constexpr std::uint32_t DETERMINISTIC_TICK_COUNT = 3U;
constexpr std::uint32_t TIMER_TICK_COUNT = 4U;
constexpr std::size_t LOG_CAPACITY = 16U;

class TraceRuntime final : public IHostRuntime
{
public:
    HostError Start(std::vector<std::string>& lifecycleTrace) override
    {
        lifecycleTrace.push_back("runtime.start");
        return HostError::Success();
    }

    HostError Tick(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) override
    {
        static_cast<void>(frameIndex);
        static_cast<void>(tickTimeNanoseconds);
        lifecycleTrace.push_back("runtime.tick");
        return HostError::Success();
    }

    HostError Shutdown(std::vector<std::string>& lifecycleTrace) override
    {
        lifecycleTrace.push_back("runtime.shutdown");
        return HostError::Success();
    }
};

int Fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

int HostStartTickShutdownDeterministic()
{
    BoundedInMemoryLogSink logSink(LOG_CAPACITY);
    FixedFrameClock frameClock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime runtime;
    HeadlessHost host(frameClock, logSink);

    const HeadlessHostConfig config{DETERMINISTIC_TICK_COUNT};
    const auto result = host.Run(runtime, config);
    if (result.Status != HostStatus::Success)
    {
        return Fail("host did not return success");
    }

    if (result.TickCount != DETERMINISTIC_TICK_COUNT)
    {
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

    if (result.LifecycleTrace != expectedTrace)
    {
        return Fail("host lifecycle trace did not match expected order");
    }

    if (result.AllocationBytesStatus != PlatformPerformanceSignal::AllocationBytesStatus)
    {
        return Fail("allocation/bytes deferral marker is missing");
    }

    if (logSink.Events().empty())
    {
        return Fail("recording log sink did not receive host events");
    }

    return 0;
}

int HostTimerMonotonicForFixedTicks()
{
    BoundedInMemoryLogSink logSink(LOG_CAPACITY);
    FixedFrameClock frameClock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime runtime;
    HeadlessHost host(frameClock, logSink);

    const HeadlessHostConfig config{TIMER_TICK_COUNT};
    const auto result = host.Run(runtime, config);
    if (result.TickTimesNanoseconds.size() != TIMER_TICK_COUNT)
    {
        return Fail("timer did not record every tick");
    }

    for (std::size_t index = 1U; index < result.TickTimesNanoseconds.size(); ++index)
    {
        if (result.TickTimesNanoseconds[index] <= result.TickTimesNanoseconds[index - 1U])
        {
            return Fail("timer ticks were not monotonic");
        }
    }

    if (result.TickTimesNanoseconds[0U] != FIRST_TICK_NANOSECONDS)
    {
        return Fail("first timer tick was unexpected");
    }

    if (result.TickTimesNanoseconds[3U] != FIRST_TICK_NANOSECONDS + (STEP_NANOSECONDS * 3U))
    {
        return Fail("fixed timer step was unexpected");
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
    if (testName == TEST_HOST)
    {
        return HostStartTickShutdownDeterministic();
    }

    if (testName == TEST_TIMER)
    {
        return HostTimerMonotonicForFixedTicks();
    }

    return Fail("unknown test name");
}
