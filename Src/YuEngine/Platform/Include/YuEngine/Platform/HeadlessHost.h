// Module: YuEngine Platform
// File: Src/YuEngine/Platform/Include/YuEngine/Platform/HeadlessHost.h

#pragma once

#include "YuEngine/Diagnostics/ILogSink.h"
#include "YuEngine/Platform/HeadlessHostConfig.h"
#include "YuEngine/Platform/HostRunResult.h"
#include "YuEngine/Platform/IFrameClock.h"
#include "YuEngine/Platform/IHostRuntime.h"

namespace yuengine::platform {
class HeadlessHost final {
public:
    /**
     * @comment Constructs a HeadlessHost instance.
     * @param frame_clock Frame clock updated by the function.
     * @param log_sink Log sink updated by the function.
     */
    HeadlessHost(IFrameClock& frame_clock, diagnostics::ILogSink& log_sink);

    /**
     * @comment Runs the host loop.
     * @param runtime Runtime updated by the function.
     * @param config Input configuration.
     * @return Explicit operation result.
     */
    HostRunResult Run(IHostRuntime& runtime, const HeadlessHostConfig& config);

private:
    IFrameClock& frame_clock_;
    diagnostics::ILogSink& log_sink_;
};
}
