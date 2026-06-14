#pragma once

#include "YuEngine/Memory/IMemoryTracker.h"

namespace yuengine::memory {
class DisabledMemoryTracker final : public IMemoryTracker {
public:
    MemoryAccountingResult RecordAllocation(
        MemoryOwnerId owner,
        MemoryTag tag,
        MemoryBudgetClass budget_class,
        std::size_t bytes,
        std::size_t alignment) override;
    MemoryAccountingStatus RecordFree(MemoryAllocationId allocation_id, MemoryOwnerId owner, MemoryTag tag) override;
    MemorySnapshot Snapshot() const override;
    std::uint64_t AllocationCountForBudget(MemoryBudgetClass budget_class) const override;
};
}
