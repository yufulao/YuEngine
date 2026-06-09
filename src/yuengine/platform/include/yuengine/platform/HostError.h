#pragma once

#include <string>

namespace yuengine::platform
{
struct HostError
{
    bool Succeeded;
    std::string Message;

    static HostError Success();
    static HostError Failure(std::string message);
};
}
