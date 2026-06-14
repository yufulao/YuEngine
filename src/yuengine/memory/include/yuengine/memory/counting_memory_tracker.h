#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "yuengine/memory/i_memory_tracker.h"
#include "yuengine/memory/active_allocation_record.h"

namespace yuengine::memory {
constexpr std::size_t MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS = 64U;

class CountingMemoryTracker final : public IMemoryTracker {
public:
    CountingMemoryTracker();

    MemoryAccountingResult RecordAllocation(
        MemoryOwnerId owner,
        MemoryTag tag,
        MEMORY_BUDGET_CLASS budgetClass,
        std::size_t bytes,
        std::size_t alignment) override;
    MEMORY_ACCOUNTING_STATUS RecordFree(MemoryAllocationId allocationId, MemoryOwnerId owner, MemoryTag tag) override;
    MemorySnapshot Snapshot() const override;
    std::uint64_t AllocationCountForBudget(MEMORY_BUDGET_CLASS budgetClass) const override;

private:
    ActiveAllocationRecord* FindActiveAllocation(MemoryAllocationId allocationId);
    ActiveAllocationRecord* FindFreeAllocationRecord();
    static void ResetAllocationRecord(ActiveAllocationRecord& record);

    std::array<ActiveAllocationRecord, MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS> _activeAllocations;
    std::array<std::uint64_t, MemoryBudgetClassCount> _budgetAllocationCounts;
    MemorySnapshot _snapshot;
    std::uint64_t _nextAllocationId;
    std::size_t _activeAllocationCount;
};
}
