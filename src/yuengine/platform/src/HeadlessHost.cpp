#include "yuengine/platform/HeadlessHost.h"

#include "yuengine/diagnostics/log_level.h"
#include "yuengine/platform/PlatformPerformanceSignal.h"

#include <cstddef>

namespace yuengine::platform {
namespace {
constexpr const char* HOST_START_TRACE = "host.start";
constexpr const char* HOST_TICK_TRACE = "host.tick";
constexpr const char* HOST_SHUTDOWN_TRACE = "host.shutdown";
}

HeadlessHost::HeadlessHost(IFrameClock& frameClock, diagnostics::ILogSink& logSink)
    : _frameClock(frameClock),
      _logSink(logSink) {
}

HostRunResult HeadlessHost::Run(IHostRuntime& runtime, const HeadlessHostConfig& config) {
    HostRunResult result{
        HostStatus::Success,
        0U,
        std::vector<std::uint64_t>(),
        std::vector<std::string>(),
        std::string(),
        PlatformPerformanceSignal::AllocationAccountingStatus};
    result.TickTimesNanoseconds.reserve(config.TickCount);
    result.LifecycleTrace.reserve((static_cast<std::size_t>(config.TickCount) * 2U) + 4U);

    result.LifecycleTrace.push_back(HOST_START_TRACE);
    _logSink.Write(diagnostics::LogLevel::Info, "host start");

    const HostError startError = runtime.Start(result.LifecycleTrace);
    if (!startError.Succeeded) {
        result.Status = HostStatus::StartupFailure;
        result.ErrorMessage = startError.Message;
        return result;
    }

    for (std::uint32_t frameIndex = 0U; frameIndex < config.TickCount; ++frameIndex) {
        const std::uint64_t tickTimeNanoseconds = _frameClock.NextTickNanoseconds();
        result.TickTimesNanoseconds.push_back(tickTimeNanoseconds);
        result.LifecycleTrace.push_back(HOST_TICK_TRACE);
        _logSink.Write(diagnostics::LogLevel::Info, "host tick");

        const HostError tickError = runtime.Tick(frameIndex, tickTimeNanoseconds, result.LifecycleTrace);
        if (!tickError.Succeeded) {
            result.LifecycleTrace.push_back(HOST_SHUTDOWN_TRACE);
            const HostError tickShutdownError = runtime.Shutdown(result.LifecycleTrace);
            if (!tickShutdownError.Succeeded) {
                result.Status = HostStatus::ShutdownFailure;
                result.ErrorMessage = tickShutdownError.Message;
                return result;
            }

            result.Status = HostStatus::TickFailure;
            result.ErrorMessage = tickError.Message;
            return result;
        }

        ++result.TickCount;
    }

    result.LifecycleTrace.push_back(HOST_SHUTDOWN_TRACE);
    _logSink.Write(diagnostics::LogLevel::Info, "host shutdown");

    const HostError shutdownError = runtime.Shutdown(result.LifecycleTrace);
    if (!shutdownError.Succeeded) {
        result.Status = HostStatus::ShutdownFailure;
        result.ErrorMessage = shutdownError.Message;
        return result;
    }

    return result;
}
}
