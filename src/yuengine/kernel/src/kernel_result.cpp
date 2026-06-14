#include "yuengine/kernel/kernel_result.h"

#include <utility>

namespace yuengine::kernel {
KernelResult KernelResult::Success() {
    return KernelResult{true, KernelStatus::Success, std::string()};
}

KernelResult KernelResult::Failure(KernelStatus status, std::string message) {
    return KernelResult{false, status, std::move(message)};
}
}
