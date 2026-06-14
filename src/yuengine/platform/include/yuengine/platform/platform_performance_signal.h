#pragma once

#include "yuengine/memory/memory_accounting_status.h"

namespace yuengine::platform {
using yuengine::memory::MemoryAccountingStatus;

struct PlatformPerformanceSignal {
    static constexpr MemoryAccountingStatus AllocationAccountingStatus = MemoryAccountingStatus::ExplicitlyTrackedOnly;
};
}
