#pragma once

#include "yuengine/memory/i_memory_tracker.h"

namespace yuengine::memory {
class DisabledMemoryTracker final : public IMemoryTracker {
public:
    memory_accounting_result_t RecordAllocation(
        memory_owner_id_t owner,
        memory_tag_t tag,
        MemoryBudgetClass budgetClass,
        std::size_t bytes,
        std::size_t alignment) override;
    MemoryAccountingStatus RecordFree(memory_allocation_id_t allocationId, memory_owner_id_t owner, memory_tag_t tag) override;
    memory_snapshot_t Snapshot() const override;
    std::uint64_t AllocationCountForBudget(MemoryBudgetClass budgetClass) const override;
};
}
