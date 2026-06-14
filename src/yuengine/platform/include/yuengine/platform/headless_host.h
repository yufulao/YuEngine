#pragma once

#include "yuengine/diagnostics/i_log_sink.h"
#include "yuengine/platform/headless_host_config.h"
#include "yuengine/platform/host_run_result.h"
#include "yuengine/platform/i_frame_clock.h"
#include "yuengine/platform/i_host_runtime.h"

namespace yuengine::platform {
class HeadlessHost final {
public:
    HeadlessHost(IFrameClock& frameClock, diagnostics::ILogSink& logSink);

    host_run_result_t Run(IHostRuntime& runtime, const headless_host_config_t& config);

private:
    IFrameClock& _frameClock;
    diagnostics::ILogSink& _logSink;
};
}
