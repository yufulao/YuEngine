// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/HeadlessHost.h

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
     * @comment 构造 HeadlessHost 实例。
     * @param frame_clock 函数写入的 Frame clock。
     * @param log_sink 函数写入的 Log sink。
     */
    HeadlessHost(IFrameClock& frame_clock, diagnostics::ILogSink& log_sink);

    /**
     * @comment 运行 host loop。
     * @param runtime 函数写入的 Runtime。
     * @param config 输入 configuration。
     * @return 显式操作结果。
     */
    HostRunResult Run(IHostRuntime& runtime, const HeadlessHostConfig& config);

private:
    IFrameClock& frame_clock_;
    diagnostics::ILogSink& log_sink_;
};
}
