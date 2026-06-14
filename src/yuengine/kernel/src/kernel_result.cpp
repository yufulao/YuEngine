#include "yuengine/kernel/kernel_result.h"

#include <utility>

namespace yuengine::kernel {
KernelResult KernelResult::Success() {
    return KernelResult{true, KERNEL_STATUS::Success, std::string()};
}

KernelResult KernelResult::Failure(KERNEL_STATUS status, std::string message) {
    return KernelResult{false, status, std::move(message)};
}
}
