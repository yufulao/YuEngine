// Module: YuEngine Platform
// File: Src/YuEngine/Platform/Include/YuEngine/Platform/HostError.h

#pragma once

#include <string>

namespace yuengine::platform {
struct HostError {
    bool succeeded;
    std::string message;

    /**
     * @comment Creates a successful result.
     * @return Success value.
     */
    static HostError Success();
    /**
     * @comment Creates a failed result.
     * @param message Input message text.
     * @return Failure value.
     */
    static HostError Failure(std::string message);
};
}
