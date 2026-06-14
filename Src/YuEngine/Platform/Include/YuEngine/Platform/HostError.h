#pragma once

#include <string>

namespace yuengine::platform {
struct HostError {
    bool succeeded;
    std::string message;

    static HostError Success();
    static HostError Failure(std::string message);
};
}
