#pragma once

#include <array>
#include <string>
#include <unordered_map>

#include "yuengine/memory/IMemoryTracker.h"

namespace yuengine::memory
{
class CountingMemoryTracker final : public IMemoryTracker
{
public:
    CountingMemoryTracker();

    MemoryAccountingResult RecordAllocation(
        MemoryOwnerId owner,
        MemoryTag tag,
        MemoryBudgetClass budgetClass,
        std::size_t bytes,
        std::size_t alignment) override;
    MemoryAccountingStatus RecordFree(MemoryAllocationId allocationId, MemoryOwnerId owner, MemoryTag tag) override;
    MemorySnapshot Snapshot() const override;
    std::uint64_t AllocationCountForBudget(MemoryBudgetClass budgetClass) const override;

private:
    std::unordered_map<std::uint64_t, std::size_t> _activeBytes;
    std::unordered_map<std::uint64_t, std::string> _activeOwners;
    std::unordered_map<std::uint64_t, std::string> _activeTags;
    std::array<std::uint64_t, MemoryBudgetClassCount> _budgetAllocationCounts;
    MemorySnapshot _snapshot;
    std::uint64_t _nextAllocationId;
};
}
