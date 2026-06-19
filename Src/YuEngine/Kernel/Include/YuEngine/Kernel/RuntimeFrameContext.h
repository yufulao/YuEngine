// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/RuntimeFrameContext.h

#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/ILogSink.h"
#include "YuEngine/Kernel/RuntimeFrameInputSnapshotRef.h"
#include "YuEngine/Kernel/RuntimeFrameMode.h"
#include "YuEngine/Kernel/RuntimeFramePhase.h"

namespace yuengine::kernel {
struct RuntimeFrameContext {
    std::uint32_t frame_index = 0U;
    std::uint64_t delta_time_nanoseconds = 0U;
    std::uint64_t fixed_time_nanoseconds = 0U;
    RuntimeFrameMode frame_mode = RuntimeFrameMode::Fixed;
    RuntimeFrameInputSnapshotRef input_snapshot;
    diagnostics::ILogSink* diagnostics_sink = nullptr;
    RuntimeFramePhase phase = RuntimeFramePhase::BeginFrame;
};
}
