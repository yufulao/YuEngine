#include "yuengine/diagnostics/disabled_log_sink.h"
#include "yuengine/kernel/engine_kernel.h"
#include "yuengine/kernel/kernel_host_runtime.h"
#include "yuengine/platform/fixed_frame_clock.h"
#include "yuengine/platform/headless_host.h"
#include "yuengine/platform/host_status.h"

using DisabledLogSink = yuengine::diagnostics::DisabledLogSink;
using EngineKernel = yuengine::kernel::EngineKernel;
using FixedFrameClock = yuengine::platform::FixedFrameClock;
using HeadlessHost = yuengine::platform::HeadlessHost;
using yuengine::platform::HeadlessHostConfig;
using yuengine::platform::HOST_STATUS;
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
    if (result.Status == HOST_STATUS::Success) {
        return 0;
    }

    return 1;
}
