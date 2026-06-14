#include "yuengine/platform/host_error.h"

#include <utility>

namespace yuengine::platform {
host_error_t host_error_t::Success() {
    return host_error_t{true, std::string()};
}

host_error_t host_error_t::Failure(std::string message) {
    return host_error_t{false, std::move(message)};
}
}
