#pragma once

#include <string>

#include "yuengine/kernel/KernelStatus.h"

namespace yuengine::kernel {
struct KernelResult {
    bool Succeeded;
    KernelStatus Status;
    std::string Message;

    static KernelResult Success();
    static KernelResult Failure(KernelStatus status, std::string message);
};
}
