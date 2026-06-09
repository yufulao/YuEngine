#pragma once

#include "yuengine/memory/IMemoryTracker.h"

namespace yuengine::memory
{
class DisabledMemoryTracker final : public IMemoryTracker
{
public:
    MemoryAccountingResult RecordAllocation(
        MemoryOwnerId owner,
        MemoryTag tag,
        MemoryBudgetClass budgetClass,
        std::size_t bytes,
        std::size_t alignment) override;
    MemoryAccountingStatus RecordFree(MemoryAllocationId allocationId, MemoryOwnerId owner, MemoryTag tag) override;
    MemorySnapshot Snapshot() const override;
    std::uint64_t AllocationCountForBudget(MemoryBudgetClass budgetClass) const override;
};
}
