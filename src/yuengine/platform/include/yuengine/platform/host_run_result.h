#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/platform/host_status.h"

namespace yuengine::platform {
using yuengine::memory::MemoryAccountingStatus;

struct host_run_result_t {
    HostStatus Status;
    std::uint32_t TickCount;
    std::vector<std::uint64_t> TickTimesNanoseconds;
    std::vector<std::string> LifecycleTrace;
    std::string ErrorMessage;
    MemoryAccountingStatus AllocationAccountingStatus;
};
}
