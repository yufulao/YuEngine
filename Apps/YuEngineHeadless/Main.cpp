#include "YuEngine/Diagnostics/DisabledLogSink.h"
#include "YuEngine/Kernel/EngineKernel.h"
#include "YuEngine/Kernel/KernelHostRuntime.h"
#include "YuEngine/Platform/FixedFrameClock.h"
#include "YuEngine/Platform/HeadlessHost.h"
#include "YuEngine/Platform/HostStatus.h"

using DisabledLogSink = yuengine::diagnostics::DisabledLogSink;
using EngineKernel = yuengine::kernel::EngineKernel;
using FixedFrameClock = yuengine::platform::FixedFrameClock;
using HeadlessHost = yuengine::platform::HeadlessHost;
using yuengine::platform::HeadlessHostConfig;
using yuengine::platform::HostStatus;
using KernelHostRuntime = yuengine::kernel::KernelHostRuntime;

int main() {
    constexpr std::uint64_t FIRST_TICK_NANOSECONDS = 0U;
    constexpr std::uint64_t STEP_NANOSECONDS = 16666666U;
    constexpr std::uint32_t TICK_COUNT = 1U;

    DisabledLogSink log_sink;
    FixedFrameClock frame_clock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    EngineKernel kernel;
    KernelHostRuntime runtime(kernel);
    HeadlessHost host(frame_clock, log_sink);

    const HeadlessHostConfig config{TICK_COUNT};
    const auto result = host.Run(runtime, config);
    if (result.status == HostStatus::Success) {
        return 0;
    }

    return 1;
}
