#include "yuengine/kernel/kernel_result.h"

#include <utility>

namespace yuengine::kernel {
kernel_result_t kernel_result_t::Success() {
    return kernel_result_t{true, KernelStatus::Success, std::string()};
}

kernel_result_t kernel_result_t::Failure(KernelStatus status, std::string message) {
    return kernel_result_t{false, status, std::move(message)};
}
}
