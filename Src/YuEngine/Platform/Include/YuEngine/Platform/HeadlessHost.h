#pragma once

#include "YuEngine/Diagnostics/ILogSink.h"
#include "YuEngine/Platform/HeadlessHostConfig.h"
#include "YuEngine/Platform/HostRunResult.h"
#include "YuEngine/Platform/IFrameClock.h"
#include "YuEngine/Platform/IHostRuntime.h"

namespace yuengine::platform {
class HeadlessHost final {
public:
    HeadlessHost(IFrameClock& frameClock, diagnostics::ILogSink& logSink);

    HostRunResult Run(IHostRuntime& runtime, const HeadlessHostConfig& config);

private:
    IFrameClock& frame_clock_;
    diagnostics::ILogSink& log_sink_;
};
}
