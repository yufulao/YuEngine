#include "yuengine/memory/disabled_memory_tracker.h"

namespace yuengine::memory {
memory_accounting_result_t DisabledMemoryTracker::RecordAllocation(
    memory_owner_id_t owner,
    memory_tag_t tag,
    MemoryBudgetClass budgetClass,
    std::size_t bytes,
    std::size_t alignment) {
    static_cast<void>(owner);
    static_cast<void>(tag);
    static_cast<void>(budgetClass);
    static_cast<void>(bytes);
    static_cast<void>(alignment);
    return memory_accounting_result_t::Success(memory_allocation_id_t{0U});
}

MemoryAccountingStatus DisabledMemoryTracker::RecordFree(memory_allocation_id_t allocationId, memory_owner_id_t owner, memory_tag_t tag) {
    static_cast<void>(allocationId);
    static_cast<void>(owner);
    static_cast<void>(tag);
    return MemoryAccountingStatus::Success;
}

memory_snapshot_t DisabledMemoryTracker::Snapshot() const {
    return memory_snapshot_t{0U, 0U, 0U, 0U, 0U};
}

std::uint64_t DisabledMemoryTracker::AllocationCountForBudget(MemoryBudgetClass budgetClass) const {
    static_cast<void>(budgetClass);
    return 0U;
}
}
