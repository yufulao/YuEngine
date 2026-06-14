#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Memory/IMemoryTracker.h"
#include "YuEngine/Memory/ActiveAllocationRecord.h"

namespace yuengine::memory {
constexpr std::size_t MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS = 64U;

class CountingMemoryTracker final : public IMemoryTracker {
public:
    CountingMemoryTracker();

    MemoryAccountingResult RecordAllocation(
        MemoryOwnerId owner,
        MemoryTag tag,
        MemoryBudgetClass budget_class,
        std::size_t bytes,
        std::size_t alignment) override;
    MemoryAccountingStatus RecordFree(MemoryAllocationId allocation_id, MemoryOwnerId owner, MemoryTag tag) override;
    MemorySnapshot Snapshot() const override;
    std::uint64_t AllocationCountForBudget(MemoryBudgetClass budget_class) const override;

private:
    ActiveAllocationRecord* FindActiveAllocation(MemoryAllocationId allocation_id);
    ActiveAllocationRecord* FindFreeAllocationRecord();
    static void ResetAllocationRecord(ActiveAllocationRecord& record);

    std::array<ActiveAllocationRecord, MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS> active_allocations_;
    std::array<std::uint64_t, MEMORY_BUDGET_CLASS_COUNT> budget_allocation_counts_;
    MemorySnapshot snapshot_;
    std::uint64_t next_allocation_id_;
    std::size_t active_allocation_count_;
};
}
