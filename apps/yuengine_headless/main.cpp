#include "yuengine/diagnostics/DisabledLogSink.h"
#include "yuengine/kernel/EngineKernel.h"
#include "yuengine/kernel/KernelHostRuntime.h"
#include "yuengine/platform/FixedFrameClock.h"
#include "yuengine/platform/HeadlessHost.h"
#include "yuengine/platform/HostStatus.h"

using DisabledLogSink = yuengine::diagnostics::DisabledLogSink;
using EngineKernel = yuengine::kernel::EngineKernel;
using FixedFrameClock = yuengine::platform::FixedFrameClock;
using HeadlessHost = yuengine::platform::HeadlessHost;
using HeadlessHostConfig = yuengine::platform::HeadlessHostConfig;
using HostStatus = yuengine::platform::HostStatus;
using KernelHostRuntime = yuengine::kernel::KernelHostRuntime;

int main() {
    constexpr std::uint64_t FIRST_TICK_NANOSECONDS = 0U;
    constexpr std::uint64_t STEP_NANOSECONDS = 16666666U;
    constexpr std::uint32_t TICK_COUNT = 1U;

    DisabledLogSink logSink;
    FixedFrameClock frameClock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    EngineKernel kernel;
    KernelHostRuntime runtime(kernel);
    HeadlessHost host(frameClock, logSink);

    const HeadlessHostConfig config{TICK_COUNT};
    const auto result = host.Run(runtime, config);
    if (result.Status == HostStatus::Success) {
        return 0;
    }

    return 1;
}
