// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/RuntimeAppRunResult.h

#pragma once

#include <cstdint>

#include "YuEngine/Kernel/KernelStatus.h"
#include "YuEngine/Kernel/RuntimeAppStatus.h"
#include "YuEngine/Kernel/RuntimeFrameContext.h"

namespace yuengine::kernel {
struct RuntimeAppRunResult {
    bool succeeded = false;
    RuntimeAppStatus status = RuntimeAppStatus::InvalidDescriptor;
    KernelStatus kernel_status = KernelStatus::Success;
    KernelStatus shutdown_kernel_status = KernelStatus::Success;
    std::uint32_t completed_frame_count = 0U;
    RuntimeFrameContext last_frame_context;
};
}
