#include "yuengine/memory/CountingMemoryTracker.h"

#include <string_view>

namespace yuengine::memory
{
namespace
{
bool IsPowerOfTwo(std::size_t value)
{
    if (value == 0U)
    {
        return false;
    }

    return (value & (value - 1U)) == 0U;
}

template <std::size_t Capacity>
bool CopyFixedText(std::string_view source, std::array<char, Capacity>& destination, std::size_t& length)
{
    if (source.size() > Capacity)
    {
        return false;
    }

    for (std::size_t index = 0U; index < Capacity; ++index)
    {
        destination[index] = 0;
    }

    for (std::size_t index = 0U; index < source.size(); ++index)
    {
        destination[index] = source[index];
    }

    length = source.size();
    return true;
}

template <std::size_t Capacity>
bool FixedTextEquals(const std::array<char, Capacity>& stored, std::size_t storedLength, std::string_view candidate)
{
    if (candidate.size() != storedLength)
    {
        return false;
    }

    for (std::size_t index = 0U; index < storedLength; ++index)
    {
        if (stored[index] != candidate[index])
        {
            return false;
        }
    }

    return true;
}
}

CountingMemoryTracker::CountingMemoryTracker()
    : _activeAllocations(),
      _budgetAllocationCounts{},
      _snapshot{0U, 0U, 0U, 0U, 0U},
      _nextAllocationId(1U),
      _activeAllocationCount(0U)
{
}

MemoryAccountingResult CountingMemoryTracker::RecordAllocation(
    MemoryOwnerId owner,
    MemoryTag tag,
    MemoryBudgetClass budgetClass,
    std::size_t bytes,
    std::size_t alignment)
{
    if (!owner.IsValid())
    {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidOwner);
    }

    if (!tag.IsValid())
    {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidTag);
    }

    if (bytes == 0U)
    {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidSize);
    }

    if (!IsPowerOfTwo(alignment))
    {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidAlignment);
    }

    if (!IsValidMemoryBudgetClass(budgetClass))
    {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidBudgetClass);
    }

    if (owner.Value.size() > MAX_MEMORY_OWNER_ID_BYTES)
    {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidOwner);
    }

    if (tag.Value.size() > MAX_MEMORY_TAG_BYTES)
    {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidTag);
    }

    if (IsHotMemoryBudgetClass(budgetClass))
    {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::BudgetExceeded);
    }

    ActiveAllocationRecord* record = FindFreeAllocationRecord();
    if (record == nullptr)
    {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::CapacityExceeded);
    }

    const MemoryAllocationId allocationId{_nextAllocationId};
    ++_nextAllocationId;

    if (!CopyFixedText(owner.Value, record->Owner, record->OwnerLength))
    {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidOwner);
    }

    if (!CopyFixedText(tag.Value, record->Tag, record->TagLength))
    {
        ResetAllocationRecord(*record);
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidTag);
    }

    record->IsActive = true;
    record->AllocationId = allocationId;
    record->Bytes = bytes;
    ++_activeAllocationCount;

    ++_snapshot.AllocationCount;
    _snapshot.RetainedBytes += bytes;
    if (_snapshot.RetainedBytes > _snapshot.PeakRetainedBytes)
    {
        _snapshot.PeakRetainedBytes = _snapshot.RetainedBytes;
    }

    _snapshot.LeakCount = _activeAllocationCount;
    ++_budgetAllocationCounts[MemoryBudgetClassIndex(budgetClass)];
    return MemoryAccountingResult::Success(allocationId);
}

MemoryAccountingStatus CountingMemoryTracker::RecordFree(MemoryAllocationId allocationId, MemoryOwnerId owner, MemoryTag tag)
{
    ActiveAllocationRecord* record = FindActiveAllocation(allocationId);
    if (record == nullptr)
    {
        return MemoryAccountingStatus::UnmatchedFree;
    }

    if (!FixedTextEquals(record->Owner, record->OwnerLength, owner.Value))
    {
        return MemoryAccountingStatus::OwnerTagMismatch;
    }

    if (!FixedTextEquals(record->Tag, record->TagLength, tag.Value))
    {
        return MemoryAccountingStatus::OwnerTagMismatch;
    }

    const std::size_t bytes = record->Bytes;
    if (bytes > _snapshot.RetainedBytes)
    {
        return MemoryAccountingStatus::UnmatchedFree;
    }

    _snapshot.RetainedBytes -= bytes;
    ++_snapshot.FreeCount;

    ResetAllocationRecord(*record);
    --_activeAllocationCount;
    _snapshot.LeakCount = _activeAllocationCount;
    return MemoryAccountingStatus::Success;
}

MemorySnapshot CountingMemoryTracker::Snapshot() const
{
    return _snapshot;
}

std::uint64_t CountingMemoryTracker::AllocationCountForBudget(MemoryBudgetClass budgetClass) const
{
    if (!IsValidMemoryBudgetClass(budgetClass))
    {
        return 0U;
    }

    return _budgetAllocationCounts[MemoryBudgetClassIndex(budgetClass)];
}

ActiveAllocationRecord* CountingMemoryTracker::FindActiveAllocation(MemoryAllocationId allocationId)
{
    for (ActiveAllocationRecord& record : _activeAllocations)
    {
        if (record.IsActive && record.AllocationId.Value == allocationId.Value)
        {
            return &record;
        }
    }

    return nullptr;
}

ActiveAllocationRecord* CountingMemoryTracker::FindFreeAllocationRecord()
{
    for (ActiveAllocationRecord& record : _activeAllocations)
    {
        if (!record.IsActive)
        {
            return &record;
        }
    }

    return nullptr;
}

void CountingMemoryTracker::ResetAllocationRecord(ActiveAllocationRecord& record)
{
    record = ActiveAllocationRecord{};
}
}
