#pragma once

#include "yuengine/memory/memory_accounting_status.h"

namespace yuengine::platform {
using MemoryAccountingStatus = yuengine::memory::MemoryAccountingStatus;

struct platform_performance_signal_t {
    static constexpr MemoryAccountingStatus AllocationAccountingStatus = MemoryAccountingStatus::ExplicitlyTrackedOnly;
};
}
