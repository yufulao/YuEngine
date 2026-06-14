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

    memory_accounting_result_t RecordAllocation(
        memory_owner_id_t owner,
        memory_tag_t tag,
        MEMORY_BUDGET_CLASS budgetClass,
        std::size_t bytes,
        std::size_t alignment) override;
    MEMORY_ACCOUNTING_STATUS RecordFree(memory_allocation_id_t allocationId, memory_owner_id_t owner, memory_tag_t tag) override;
    memory_snapshot_t Snapshot() const override;
    std::uint64_t AllocationCountForBudget(MEMORY_BUDGET_CLASS budgetClass) const override;

private:
    active_allocation_record_t* FindActiveAllocation(memory_allocation_id_t allocationId);
    active_allocation_record_t* FindFreeAllocationRecord();
    static void ResetAllocationRecord(active_allocation_record_t& record);

    std::array<active_allocation_record_t, MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS> _activeAllocations;
    std::array<std::uint64_t, MemoryBudgetClassCount> _budgetAllocationCounts;
    memory_snapshot_t _snapshot;
    std::uint64_t _nextAllocationId;
    std::size_t _activeAllocationCount;
};
}
