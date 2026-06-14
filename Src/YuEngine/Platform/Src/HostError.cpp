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
