// Module: YuEngine Platform
// File: Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformPerformanceSignal.h

#pragma once

#include "YuEngine/Memory/MemoryAccountingStatus.h"

namespace yuengine::platform {
using yuengine::memory::MemoryAccountingStatus;

struct PlatformPerformanceSignal {
    static constexpr MemoryAccountingStatus ALLOCATION_ACCOUNTING_STATUS = MemoryAccountingStatus::ExplicitlyTrackedOnly;
};
}
