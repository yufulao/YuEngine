#include "yuengine/memory/CountingMemoryTracker.h"

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
}

CountingMemoryTracker::CountingMemoryTracker()
    : _activeBytes(),
      _activeOwners(),
      _activeTags(),
      _budgetAllocationCounts{},
      _snapshot{0U, 0U, 0U, 0U, 0U},
      _nextAllocationId(1U)
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

    if (IsHotMemoryBudgetClass(budgetClass))
    {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::BudgetExceeded);
    }

    const MemoryAllocationId allocationId{_nextAllocationId};
    ++_nextAllocationId;

    _activeBytes.emplace(allocationId.Value, bytes);
    _activeOwners.emplace(allocationId.Value, std::string(owner.Value));
    _activeTags.emplace(allocationId.Value, std::string(tag.Value));

    ++_snapshot.AllocationCount;
    _snapshot.RetainedBytes += bytes;
    if (_snapshot.RetainedBytes > _snapshot.PeakRetainedBytes)
    {
        _snapshot.PeakRetainedBytes = _snapshot.RetainedBytes;
    }

    _snapshot.LeakCount = _activeBytes.size();
    ++_budgetAllocationCounts[MemoryBudgetClassIndex(budgetClass)];
    return MemoryAccountingResult::Success(allocationId);
}

MemoryAccountingStatus CountingMemoryTracker::RecordFree(MemoryAllocationId allocationId, MemoryOwnerId owner, MemoryTag tag)
{
    const auto bytesIterator = _activeBytes.find(allocationId.Value);
    if (bytesIterator == _activeBytes.end())
    {
        return MemoryAccountingStatus::UnmatchedFree;
    }

    const auto ownerIterator = _activeOwners.find(allocationId.Value);
    const auto tagIterator = _activeTags.find(allocationId.Value);
    if (ownerIterator == _activeOwners.end())
    {
        return MemoryAccountingStatus::UnmatchedFree;
    }

    if (tagIterator == _activeTags.end())
    {
        return MemoryAccountingStatus::UnmatchedFree;
    }

    if (ownerIterator->second != owner.Value)
    {
        return MemoryAccountingStatus::OwnerTagMismatch;
    }

    if (tagIterator->second != tag.Value)
    {
        return MemoryAccountingStatus::OwnerTagMismatch;
    }

    const std::size_t bytes = bytesIterator->second;
    if (bytes > _snapshot.RetainedBytes)
    {
        return MemoryAccountingStatus::UnmatchedFree;
    }

    _snapshot.RetainedBytes -= bytes;
    ++_snapshot.FreeCount;

    _activeBytes.erase(bytesIterator);
    _activeOwners.erase(ownerIterator);
    _activeTags.erase(tagIterator);
    _snapshot.LeakCount = _activeBytes.size();
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
}
