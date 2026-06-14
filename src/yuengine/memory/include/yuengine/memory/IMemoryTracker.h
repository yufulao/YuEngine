#pragma once

#include <cstddef>
#include <cstdint>

#include "yuengine/memory/MemoryAccountingResult.h"
#include "yuengine/memory/MemoryAccountingStatus.h"
#include "yuengine/memory/MemoryAllocationId.h"
#include "yuengine/memory/MemoryBudgetClass.h"
#include "yuengine/memory/MemoryOwnerId.h"
#include "yuengine/memory/MemorySnapshot.h"
#include "yuengine/memory/MemoryTag.h"

namespace yuengine::memory {
class IMemoryTracker {
public:
    virtual ~IMemoryTracker() = default;

    virtual MemoryAccountingResult RecordAllocation(
        MemoryOwnerId owner,
        MemoryTag tag,
        MemoryBudgetClass budgetClass,
        std::size_t bytes,
        std::size_t alignment) = 0;
    virtual MemoryAccountingStatus RecordFree(MemoryAllocationId allocationId, MemoryOwnerId owner, MemoryTag tag) = 0;
    virtual MemorySnapshot Snapshot() const = 0;
    virtual std::uint64_t AllocationCountForBudget(MemoryBudgetClass budgetClass) const = 0;
};
}
