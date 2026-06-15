// Module: YuEngine Memory
// File: Src/YuEngine/Memory/Include/YuEngine/Memory/IMemoryTracker.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingResult.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Memory/MemoryAllocationId.h"
#include "YuEngine/Memory/MemoryBudgetClass.h"
#include "YuEngine/Memory/MemoryOwnerId.h"
#include "YuEngine/Memory/MemorySnapshot.h"
#include "YuEngine/Memory/MemoryTag.h"

namespace yuengine::memory {
class IMemoryTracker {
public:
    virtual ~IMemoryTracker() = default;

    /**
     * @comment Records allocation.
     * @param owner Input owner.
     * @param tag Input tag.
     * @param budget_class Input budget class.
     * @param bytes Input byte count or byte payload.
     * @param alignment Input alignment.
     * @return Explicit operation result.
     */
    virtual MemoryAccountingResult RecordAllocation(
        MemoryOwnerId owner,
        MemoryTag tag,
        MemoryBudgetClass budget_class,
        std::size_t bytes,
        std::size_t alignment) = 0;
    /**
     * @comment Records free.
     * @param allocation_id Input allocation id.
     * @param owner Input owner.
     * @param tag Input tag.
     * @return Explicit operation status.
     */
    virtual MemoryAccountingStatus RecordFree(MemoryAllocationId allocation_id, MemoryOwnerId owner, MemoryTag tag) = 0;
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    virtual MemorySnapshot Snapshot() const = 0;
    /**
     * @comment Returns count for budget.
     * @param budget_class Input budget class.
     * @return Allocation count for budget value.
     */
    virtual std::uint64_t AllocationCountForBudget(MemoryBudgetClass budget_class) const = 0;
};
}
