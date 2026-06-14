#pragma once

#include <string>

#include "yuengine/kernel/kernel_status.h"

namespace yuengine::kernel {
struct kernel_result_t {
    bool Succeeded;
    KernelStatus Status;
    std::string Message;

    static kernel_result_t Success();
    static kernel_result_t Failure(KernelStatus status, std::string message);
};
}
