#pragma once

#include <cstddef>
#include <cstdint>

#include "yuengine/memory/memory_accounting_result.h"
#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/memory/memory_allocation_id.h"
#include "yuengine/memory/memory_budget_class.h"
#include "yuengine/memory/memory_owner_id.h"
#include "yuengine/memory/memory_snapshot.h"
#include "yuengine/memory/memory_tag.h"

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
