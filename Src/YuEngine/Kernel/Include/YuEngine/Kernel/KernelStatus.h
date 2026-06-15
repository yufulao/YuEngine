// Module: YuEngine Kernel
// File: Src/YuEngine/Kernel/Include/YuEngine/Kernel/KernelStatus.h

#pragma once

namespace yuengine::kernel {
enum class KernelStatus {
    Success,
    DuplicateModule,
    DependencyFailure,
    MissingService,
    DuplicateService,
    StartupFailure,
    UpdateFailure,
    ShutdownFailure,
    InvalidLifecycle
};
}
