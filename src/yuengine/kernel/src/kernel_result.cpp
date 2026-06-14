#include "yuengine/kernel/kernel_result.h"

#include <utility>

namespace yuengine::kernel {
kernel_result_t kernel_result_t::Success() {
    return kernel_result_t{true, KERNEL_STATUS::Success, std::string()};
}

kernel_result_t kernel_result_t::Failure(KERNEL_STATUS status, std::string message) {
    return kernel_result_t{false, status, std::move(message)};
}
}
