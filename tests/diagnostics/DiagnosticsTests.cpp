#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "yuengine/diagnostics/BoundedInMemoryLogSink.h"
#include "yuengine/diagnostics/DisabledLogSink.h"
#include "yuengine/platform/FixedFrameClock.h"
#include "yuengine/platform/HeadlessHost.h"
#include "yuengine/platform/HostStatus.h"
#include "yuengine/platform/IHostRuntime.h"

using BoundedInMemoryLogSink = yuengine::diagnostics::BoundedInMemoryLogSink;
using DisabledLogSink = yuengine::diagnostics::DisabledLogSink;
using FixedFrameClock = yuengine::platform::FixedFrameClock;
using HeadlessHost = yuengine::platform::HeadlessHost;
using HeadlessHostConfig = yuengine::platform::HeadlessHostConfig;
using HostError = yuengine::platform::HostError;
using HostStatus = yuengine::platform::HostStatus;
using IHostRuntime = yuengine::platform::IHostRuntime;

namespace
{
constexpr const char* TEST_DISABLED_LOGGING = "Logging_DisabledSink_DoesNotChangeBehavior";
constexpr std::uint64_t FIRST_TICK_NANOSECONDS = 2000U;
constexpr std::uint64_t STEP_NANOSECONDS = 32U;
constexpr std::uint32_t TICK_COUNT = 2U;
constexpr std::size_t LOG_CAPACITY = 8U;

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

int LoggingDisabledSinkDoesNotChangeBehavior()
{
    BoundedInMemoryLogSink recordingLogSink(LOG_CAPACITY);
    FixedFrameClock recordingClock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime recordingRuntime;
    HeadlessHost recordingHost(recordingClock, recordingLogSink);

    const HeadlessHostConfig config{TICK_COUNT};
    const auto recordingResult = recordingHost.Run(recordingRuntime, config);
    if (recordingResult.Status != HostStatus::Success)
    {
        return Fail("recording host run failed");
    }

    DisabledLogSink disabledLogSink;
    FixedFrameClock disabledClock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime disabledRuntime;
    HeadlessHost disabledHost(disabledClock, disabledLogSink);

    const auto disabledResult = disabledHost.Run(disabledRuntime, config);
    if (disabledResult.Status != HostStatus::Success)
    {
        return Fail("disabled host run failed");
    }

    if (recordingResult.LifecycleTrace != disabledResult.LifecycleTrace)
    {
        return Fail("disabled logging changed lifecycle behavior");
    }

    if (recordingResult.TickTimesNanoseconds != disabledResult.TickTimesNanoseconds)
    {
        return Fail("disabled logging changed timer behavior");
    }

    if (recordingLogSink.Events().empty())
    {
        return Fail("recording logging did not observe host events");
    }

    if (recordingLogSink.DroppedCount() != 0U)
    {
        return Fail("bounded in-memory sink dropped unexpected events");
    }

    if (disabledLogSink.IsEnabled())
    {
        return Fail("disabled sink reported enabled");
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
    if (testName == TEST_DISABLED_LOGGING)
    {
        return LoggingDisabledSinkDoesNotChangeBehavior();
    }

    return Fail("unknown test name");
}
