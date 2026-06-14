#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Platform/HostStatus.h"

namespace yuengine::platform {
using yuengine::memory::MemoryAccountingStatus;

struct HostRunResult {
    HostStatus Status;
    std::uint32_t TickCount;
    std::vector<std::uint64_t> TickTimesNanoseconds;
    std::vector<std::string> LifecycleTrace;
    std::string ErrorMessage;
    MemoryAccountingStatus AllocationAccountingStatus;
};
}
