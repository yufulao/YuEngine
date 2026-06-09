#include "yuengine/memory/DisabledMemoryTracker.h"

namespace yuengine::memory
{
MemoryAccountingResult DisabledMemoryTracker::RecordAllocation(
    MemoryOwnerId owner,
    MemoryTag tag,
    MemoryBudgetClass budgetClass,
    std::size_t bytes,
    std::size_t alignment)
{
    static_cast<void>(owner);
    static_cast<void>(tag);
    static_cast<void>(budgetClass);
    static_cast<void>(bytes);
    static_cast<void>(alignment);
    return MemoryAccountingResult::Success(MemoryAllocationId{0U});
}

MemoryAccountingStatus DisabledMemoryTracker::RecordFree(MemoryAllocationId allocationId, MemoryOwnerId owner, MemoryTag tag)
{
    static_cast<void>(allocationId);
    static_cast<void>(owner);
    static_cast<void>(tag);
    return MemoryAccountingStatus::Success;
}

MemorySnapshot DisabledMemoryTracker::Snapshot() const
{
    return MemorySnapshot{0U, 0U, 0U, 0U, 0U};
}

std::uint64_t DisabledMemoryTracker::AllocationCountForBudget(MemoryBudgetClass budgetClass) const
{
    static_cast<void>(budgetClass);
    return 0U;
}
}
