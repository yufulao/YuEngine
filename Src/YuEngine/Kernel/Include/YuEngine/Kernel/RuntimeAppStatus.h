// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/RuntimeAppStatus.h

#pragma once

namespace yuengine::kernel {
enum class RuntimeAppStatus {
    Success,
    InvalidDescriptor,
    AlreadyRunning,
    KernelStartupFailure,
    KernelUpdateFailure,
    KernelShutdownFailure
};
}
