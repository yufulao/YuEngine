#pragma once

#include "yuengine/memory/memory_accounting_status.h"

namespace yuengine::platform {
using yuengine::memory::MEMORY_ACCOUNTING_STATUS;

struct platform_performance_signal_t {
    static constexpr MEMORY_ACCOUNTING_STATUS AllocationAccountingStatus = MEMORY_ACCOUNTING_STATUS::ExplicitlyTrackedOnly;
};
}
