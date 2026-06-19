// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/RuntimeAppSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Kernel/KernelStatus.h"
#include "YuEngine/Kernel/RuntimeAppStatus.h"
#include "YuEngine/Kernel/RuntimeFramePhase.h"

namespace yuengine::kernel {
struct RuntimeAppSnapshot {
    RuntimeAppStatus status = RuntimeAppStatus::Success;
    KernelStatus kernel_status = KernelStatus::Success;
    KernelStatus shutdown_kernel_status = KernelStatus::Success;
    RuntimeFramePhase current_phase = RuntimeFramePhase::BeginFrame;
    std::uint32_t completed_frame_count = 0U;
    bool running = false;
};
}
