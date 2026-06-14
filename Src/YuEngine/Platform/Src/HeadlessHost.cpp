#include "YuEngine/Platform/HeadlessHost.h"

#include "YuEngine/Diagnostics/LogLevel.h"
#include "YuEngine/Platform/PlatformPerformanceSignal.h"

#include <cstddef>

namespace yuengine::platform {
namespace {
constexpr const char* HOST_START_TRACE = "host.start";
constexpr const char* HOST_TICK_TRACE = "host.tick";
constexpr const char* HOST_SHUTDOWN_TRACE = "host.shutdown";
}

HeadlessHost::HeadlessHost(IFrameClock& frameClock, diagnostics::ILogSink& logSink)
    : frame_clock_(frameClock),
      log_sink_(logSink) {
}

HostRunResult HeadlessHost::Run(IHostRuntime& runtime, const HeadlessHostConfig& config) {
    HostRunResult result{
        HostStatus::Success,
        0U,
        std::vector<std::uint64_t>(),
        std::vector<std::string>(),
        std::string(),
        PlatformPerformanceSignal::AllocationAccountingStatus};
    result.tick_times_nanoseconds.reserve(config.tick_count);
    result.lifecycle_trace.reserve((static_cast<std::size_t>(config.tick_count) * 2U) + 4U);

    result.lifecycle_trace.push_back(HOST_START_TRACE);
    log_sink_.Write(diagnostics::LogLevel::Info, "host start");

    const HostError startError = runtime.Start(result.lifecycle_trace);
    if (!startError.succeeded) {
        result.status = HostStatus::StartupFailure;
        result.error_message = startError.message;
        return result;
    }

    for (std::uint32_t frameIndex = 0U; frameIndex < config.tick_count; ++frameIndex) {
        const std::uint64_t tickTimeNanoseconds = frame_clock_.NextTickNanoseconds();
        result.tick_times_nanoseconds.push_back(tickTimeNanoseconds);
        result.lifecycle_trace.push_back(HOST_TICK_TRACE);
        log_sink_.Write(diagnostics::LogLevel::Info, "host tick");

        const HostError tickError = runtime.Tick(frameIndex, tickTimeNanoseconds, result.lifecycle_trace);
        if (!tickError.succeeded) {
            result.lifecycle_trace.push_back(HOST_SHUTDOWN_TRACE);
            const HostError tickShutdownError = runtime.Shutdown(result.lifecycle_trace);
            if (!tickShutdownError.succeeded) {
                result.status = HostStatus::ShutdownFailure;
                result.error_message = tickShutdownError.message;
                return result;
            }

            result.status = HostStatus::TickFailure;
            result.error_message = tickError.message;
            return result;
        }

        ++result.tick_count;
    }

    result.lifecycle_trace.push_back(HOST_SHUTDOWN_TRACE);
    log_sink_.Write(diagnostics::LogLevel::Info, "host shutdown");

    const HostError shutdownError = runtime.Shutdown(result.lifecycle_trace);
    if (!shutdownError.succeeded) {
        result.status = HostStatus::ShutdownFailure;
        result.error_message = shutdownError.message;
        return result;
    }

    return result;
}
}
