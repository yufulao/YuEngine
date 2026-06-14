#pragma once

#include <string>

namespace yuengine::platform {
struct host_error_t {
    bool Succeeded;
    std::string Message;

    static host_error_t Success();
    static host_error_t Failure(std::string message);
};
}
