#pragma once

#include <string>

#include "yuengine/kernel/kernel_status.h"

namespace yuengine::kernel {
struct kernel_result_t {
    bool Succeeded;
    KERNEL_STATUS Status;
    std::string Message;

    static kernel_result_t Success();
    static kernel_result_t Failure(KERNEL_STATUS status, std::string message);
};
}
