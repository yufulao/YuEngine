#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "yuengine/memory/MemoryAccountingStatus.h"
#include "yuengine/platform/HostStatus.h"

namespace yuengine::platform
{
using MemoryAccountingStatus = yuengine::memory::MemoryAccountingStatus;

struct HostRunResult
{
    HostStatus Status;
    std::uint32_t TickCount;
    std::vector<std::uint64_t> TickTimesNanoseconds;
    std::vector<std::string> LifecycleTrace;
    std::string ErrorMessage;
    MemoryAccountingStatus AllocationAccountingStatus;
};
}
