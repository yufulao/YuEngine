// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Src/HostError.cpp

#include "YuEngine/Platform/HostError.h"

#include <utility>

namespace yuengine::platform {
HostError HostError::Success() {
    return HostError{true, std::string()};
}

HostError HostError::Failure(std::string message) {
    return HostError{false, std::move(message)};
}
}
