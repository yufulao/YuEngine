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

HeadlessHost::HeadlessHost(IFrameClock& frame_clock, diagnostics::ILogSink& log_sink)
    : frame_clock_(frame_clock),
      log_sink_(log_sink) {
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

    const HostError start_error = runtime.Start(result.lifecycle_trace);
    if (!start_error.succeeded) {
        result.status = HostStatus::StartupFailure;
        result.error_message = start_error.message;
        return result;
    }

    for (std::uint32_t frame_index = 0U; frame_index < config.tick_count; ++frame_index) {
        const std::uint64_t tick_time_nanoseconds = frame_clock_.NextTickNanoseconds();
        result.tick_times_nanoseconds.push_back(tick_time_nanoseconds);
        result.lifecycle_trace.push_back(HOST_TICK_TRACE);
        log_sink_.Write(diagnostics::LogLevel::Info, "host tick");

        const HostError tick_error = runtime.Tick(frame_index, tick_time_nanoseconds, result.lifecycle_trace);
        if (!tick_error.succeeded) {
            result.lifecycle_trace.push_back(HOST_SHUTDOWN_TRACE);
            const HostError tick_shutdown_error = runtime.Shutdown(result.lifecycle_trace);
            if (!tick_shutdown_error.succeeded) {
                result.status = HostStatus::ShutdownFailure;
                result.error_message = tick_shutdown_error.message;
                return result;
            }

            result.status = HostStatus::TickFailure;
            result.error_message = tick_error.message;
            return result;
        }

        ++result.tick_count;
    }

    result.lifecycle_trace.push_back(HOST_SHUTDOWN_TRACE);
    log_sink_.Write(diagnostics::LogLevel::Info, "host shutdown");

    const HostError shutdown_error = runtime.Shutdown(result.lifecycle_trace);
    if (!shutdown_error.succeeded) {
        result.status = HostStatus::ShutdownFailure;
        result.error_message = shutdown_error.message;
        return result;
    }

    return result;
}
}
