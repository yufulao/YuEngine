// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/RuntimeAppDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/ILogSink.h"
#include "YuEngine/Kernel/RuntimeFrameInputSnapshotRef.h"
#include "YuEngine/Kernel/RuntimeFrameMode.h"

namespace yuengine::kernel {
struct RuntimeAppDesc {
    std::uint32_t frame_count = 1U;
    std::uint64_t fixed_delta_time_nanoseconds = 16666666U;
    RuntimeFrameMode frame_mode = RuntimeFrameMode::Fixed;
    RuntimeFrameInputSnapshotRef input_snapshot;
    diagnostics::ILogSink* diagnostics_sink = nullptr;
};
}
