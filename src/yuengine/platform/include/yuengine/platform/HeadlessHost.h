#pragma once

#include "yuengine/diagnostics/ILogSink.h"
#include "yuengine/platform/HeadlessHostConfig.h"
#include "yuengine/platform/HostRunResult.h"
#include "yuengine/platform/IFrameClock.h"
#include "yuengine/platform/IHostRuntime.h"

namespace yuengine::platform {
class HeadlessHost final {
public:
    HeadlessHost(IFrameClock& frameClock, diagnostics::ILogSink& logSink);

    HostRunResult Run(IHostRuntime& runtime, const HeadlessHostConfig& config);

private:
    IFrameClock& _frameClock;
    diagnostics::ILogSink& _logSink;
};
}
