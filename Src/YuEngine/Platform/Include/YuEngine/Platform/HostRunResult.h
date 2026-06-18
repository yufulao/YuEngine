// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/HostRunResult.h

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Platform/HostStatus.h"

namespace yuengine::platform {
using yuengine::memory::MemoryAccountingStatus;

struct HostRunResult {
    HostStatus status;
    std::uint32_t tick_count;
    std::vector<std::uint64_t> tick_times_nanoseconds;
    std::vector<std::string> lifecycle_trace;
    std::string error_message;
    MemoryAccountingStatus allocation_accounting_status;
};
}
