// 模块: YuEngine Memory
// 文件: Src/YuEngine/Memory/Include/YuEngine/Memory/MemorySnapshot.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Memory/ActiveAllocationRecord.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Memory/MemoryBudgetClass.h"

namespace yuengine::memory {
struct MemorySnapshot {
    std::uint64_t allocation_count = 0U;
    std::uint64_t free_count = 0U;
    std::size_t retained_bytes = 0U;
    std::size_t peak_retained_bytes = 0U;
    std::size_t leak_count = 0U;
    MemoryAccountingStatus last_status = MemoryAccountingStatus::Success;
    std::size_t allocation_capacity = 0U;
    std::size_t required_allocation_count = 0U;
    std::size_t last_allocation_capacity_entry_requested_bytes = 0U;
    std::array<char, MAX_MEMORY_OWNER_ID_BYTES> last_allocation_capacity_entry_owner{};
    std::size_t last_allocation_capacity_entry_owner_length = 0U;
    std::array<char, MAX_MEMORY_TAG_BYTES> last_allocation_capacity_entry_tag{};
    std::size_t last_allocation_capacity_entry_tag_length = 0U;
    MemoryBudgetClass last_allocation_capacity_entry_budget_class = MemoryBudgetClass::Setup;
    std::size_t last_allocation_capacity_entry_capacity = 0U;
    std::size_t last_allocation_capacity_entry_active_count = 0U;
    std::size_t last_allocation_capacity_entry_retained_bytes = 0U;

    /**
     * @comment 检查 保留的 allocations 保留.
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool HasLeaks() const {
        if (retained_bytes != 0U) {
            return true;
        }

        return leak_count != 0U;
    }
};
}
