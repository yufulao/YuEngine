// 模块: YuEngine Memory
// 文件: Src/YuEngine/Memory/Src/DisabledMemoryTracker.cpp

#include "YuEngine/Memory/DisabledMemoryTracker.h"

namespace yuengine::memory {
MemoryAccountingResult DisabledMemoryTracker::RecordAllocation(
    MemoryOwnerId owner,
    MemoryTag tag,
    MemoryBudgetClass budget_class,
    std::size_t bytes,
    std::size_t alignment) {
    static_cast<void>(owner);
    static_cast<void>(tag);
    static_cast<void>(budget_class);
    static_cast<void>(bytes);
    static_cast<void>(alignment);
    return MemoryAccountingResult::Success(MemoryAllocationId{0U});
}

MemoryAccountingStatus DisabledMemoryTracker::RecordFree(MemoryAllocationId allocation_id, MemoryOwnerId owner, MemoryTag tag) {
    static_cast<void>(allocation_id);
    static_cast<void>(owner);
    static_cast<void>(tag);
    return MemoryAccountingStatus::Success;
}

MemorySnapshot DisabledMemoryTracker::Snapshot() const {
    MemorySnapshot result{};
    result.last_status = MemoryAccountingStatus::Success;
    return result;
}

std::uint64_t DisabledMemoryTracker::AllocationCountForBudget(MemoryBudgetClass budget_class) const {
    static_cast<void>(budget_class);
    return 0U;
}
}
