#pragma once

#include <string>

#include "YuEngine/Kernel/KernelStatus.h"

namespace yuengine::kernel {
struct KernelResult {
    bool succeeded;
    KernelStatus status;
    std::string message;

    static KernelResult Success();
    static KernelResult Failure(KernelStatus status, std::string message);
};
}
