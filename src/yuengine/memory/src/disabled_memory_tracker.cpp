#include "yuengine/memory/disabled_memory_tracker.h"

namespace yuengine::memory {
MemoryAccountingResult DisabledMemoryTracker::RecordAllocation(
    MemoryOwnerId owner,
    MemoryTag tag,
    MEMORY_BUDGET_CLASS budgetClass,
    std::size_t bytes,
    std::size_t alignment) {
    static_cast<void>(owner);
    static_cast<void>(tag);
    static_cast<void>(budgetClass);
    static_cast<void>(bytes);
    static_cast<void>(alignment);
    return MemoryAccountingResult::Success(MemoryAllocationId{0U});
}

MEMORY_ACCOUNTING_STATUS DisabledMemoryTracker::RecordFree(MemoryAllocationId allocationId, MemoryOwnerId owner, MemoryTag tag) {
    static_cast<void>(allocationId);
    static_cast<void>(owner);
    static_cast<void>(tag);
    return MEMORY_ACCOUNTING_STATUS::Success;
}

MemorySnapshot DisabledMemoryTracker::Snapshot() const {
    return MemorySnapshot{0U, 0U, 0U, 0U, 0U};
}

std::uint64_t DisabledMemoryTracker::AllocationCountForBudget(MEMORY_BUDGET_CLASS budgetClass) const {
    static_cast<void>(budgetClass);
    return 0U;
}
}
