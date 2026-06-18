// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformPerformanceSignal.h

#pragma once

#include "YuEngine/Memory/MemoryAccountingStatus.h"

namespace yuengine::platform {
using yuengine::memory::MemoryAccountingStatus;

struct PlatformPerformanceSignal {
    static constexpr MemoryAccountingStatus ALLOCATION_ACCOUNTING_STATUS = MemoryAccountingStatus::ExplicitlyTrackedOnly;
};
}
