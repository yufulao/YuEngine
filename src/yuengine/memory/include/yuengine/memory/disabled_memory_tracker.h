#pragma once

#include "yuengine/memory/i_memory_tracker.h"

namespace yuengine::memory {
class DisabledMemoryTracker final : public IMemoryTracker {
public:
    MemoryAccountingResult RecordAllocation(
        MemoryOwnerId owner,
        MemoryTag tag,
        MEMORY_BUDGET_CLASS budgetClass,
        std::size_t bytes,
        std::size_t alignment) override;
    MEMORY_ACCOUNTING_STATUS RecordFree(MemoryAllocationId allocationId, MemoryOwnerId owner, MemoryTag tag) override;
    MemorySnapshot Snapshot() const override;
    std::uint64_t AllocationCountForBudget(MEMORY_BUDGET_CLASS budgetClass) const override;
};
}
