#pragma once

namespace yuengine::kernel {
enum class KERNEL_STATUS {
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
