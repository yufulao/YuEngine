// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentQuerySnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/World/WorldComponentQueryStatus.h"

namespace yuengine::world {
struct WorldComponentQuerySnapshot final {
    std::uint64_t query_count = 0U;
    std::uint64_t matched_record_count = 0U;
    std::uint32_t overflow_rejection_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    WorldComponentQueryStatus last_status = WorldComponentQueryStatus::Success;
};
}
