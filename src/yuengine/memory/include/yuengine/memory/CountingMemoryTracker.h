#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "yuengine/memory/IMemoryTracker.h"

namespace yuengine::memory
{
constexpr std::size_t MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS = 64U;
constexpr std::size_t MAX_MEMORY_OWNER_ID_BYTES = 64U;
constexpr std::size_t MAX_MEMORY_TAG_BYTES = 64U;

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
    struct ActiveAllocationRecord final
    {
        bool IsActive = false;
        MemoryAllocationId AllocationId{};
        std::size_t Bytes = 0U;
        std::array<char, MAX_MEMORY_OWNER_ID_BYTES> Owner{};
        std::size_t OwnerLength = 0U;
        std::array<char, MAX_MEMORY_TAG_BYTES> Tag{};
        std::size_t TagLength = 0U;
    };

    ActiveAllocationRecord* FindActiveAllocation(MemoryAllocationId allocationId);
    ActiveAllocationRecord* FindFreeAllocationRecord();
    static void ResetAllocationRecord(ActiveAllocationRecord& record);

    std::array<ActiveAllocationRecord, MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS> _activeAllocations;
    std::array<std::uint64_t, MemoryBudgetClassCount> _budgetAllocationCounts;
    MemorySnapshot _snapshot;
    std::uint64_t _nextAllocationId;
    std::size_t _activeAllocationCount;
};
}
