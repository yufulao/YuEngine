// Module: YuEngine Memory
// File: Src/YuEngine/Memory/Include/YuEngine/Memory/DisabledMemoryTracker.h

#pragma once

#include "YuEngine/Memory/IMemoryTracker.h"

namespace yuengine::memory {
class DisabledMemoryTracker final : public IMemoryTracker {
public:
    /**
     * @comment Records allocation.
     * @param owner Input owner.
     * @param tag Input tag.
     * @param budget_class Input budget class.
     * @param bytes Input byte count or byte payload.
     * @param alignment Input alignment.
     * @return Explicit operation result.
     */
    MemoryAccountingResult RecordAllocation(
        MemoryOwnerId owner,
        MemoryTag tag,
        MemoryBudgetClass budget_class,
        std::size_t bytes,
        std::size_t alignment) override;
    /**
     * @comment Records free.
     * @param allocation_id Input allocation id.
     * @param owner Input owner.
     * @param tag Input tag.
     * @return Explicit operation status.
     */
    MemoryAccountingStatus RecordFree(MemoryAllocationId allocation_id, MemoryOwnerId owner, MemoryTag tag) override;
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    MemorySnapshot Snapshot() const override;
    /**
     * @comment Returns count for budget.
     * @param budget_class Input budget class.
     * @return Allocation count for budget value.
     */
    std::uint64_t AllocationCountForBudget(MemoryBudgetClass budget_class) const override;
};
}
