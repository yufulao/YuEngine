#pragma once

#include <string>

#include "yuengine/kernel/kernel_status.h"

namespace yuengine::kernel {
struct KernelResult {
    bool Succeeded;
    KERNEL_STATUS Status;
    std::string Message;

    static KernelResult Success();
    static KernelResult Failure(KERNEL_STATUS status, std::string message);
};
}
