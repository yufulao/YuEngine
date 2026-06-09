#pragma once

#include "yuengine/memory/MemoryAccountingStatus.h"

namespace yuengine::platform
{
using MemoryAccountingStatus = yuengine::memory::MemoryAccountingStatus;

struct PlatformPerformanceSignal
{
    static constexpr MemoryAccountingStatus AllocationAccountingStatus = MemoryAccountingStatus::ExplicitlyTrackedOnly;
};
}
