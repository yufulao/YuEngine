// Module: YuEngine Kernel
// File: Src/YuEngine/Kernel/Include/YuEngine/Kernel/KernelResult.h

#pragma once

#include <string>

#include "YuEngine/Kernel/KernelStatus.h"

namespace yuengine::kernel {
struct KernelResult {
    bool succeeded;
    KernelStatus status;
    std::string message;

    /**
     * @comment Creates a successful result.
     * @return Explicit operation result.
     */
    static KernelResult Success();
    /**
     * @comment Creates a failed result.
     * @param status Input status.
     * @param message Input message text.
     * @return Explicit operation result.
     */
    static KernelResult Failure(KernelStatus status, std::string message);
};
}
