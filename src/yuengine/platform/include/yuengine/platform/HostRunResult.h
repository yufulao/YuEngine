#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "yuengine/platform/HostStatus.h"

namespace yuengine::platform
{
struct HostRunResult
{
    HostStatus Status;
    std::uint32_t TickCount;
    std::vector<std::uint64_t> TickTimesNanoseconds;
    std::vector<std::string> LifecycleTrace;
    std::string ErrorMessage;
    std::string AllocationBytesStatus;
};
}
