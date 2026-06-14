#include "yuengine/memory/counting_memory_tracker.h"

#include <string_view>

namespace yuengine::memory {
namespace {
bool IsPowerOfTwo(std::size_t value) {
    if (value == 0U) {
        return false;
    }

    return (value & (value - 1U)) == 0U;
}

template <std::size_t Capacity>
bool CopyFixedText(std::string_view source, std::array<char, Capacity>& destination, std::size_t& length) {
    if (source.size() > Capacity) {
        return false;
    }

    for (std::size_t index = 0U; index < Capacity; ++index) {
        destination[index] = 0;
    }

    for (std::size_t index = 0U; index < source.size(); ++index) {
        destination[index] = source[index];
    }

    length = source.size();
    return true;
}

template <std::size_t Capacity>
bool FixedTextEquals(const std::array<char, Capacity>& stored, std::size_t storedLength, std::string_view candidate) {
    if (candidate.size() != storedLength) {
        return false;
    }

    for (std::size_t index = 0U; index < storedLength; ++index) {
        if (stored[index] != candidate[index]) {
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
      _activeAllocationCount(0U) {
}

memory_accounting_result_t CountingMemoryTracker::RecordAllocation(
    memory_owner_id_t owner,
    memory_tag_t tag,
    MEMORY_BUDGET_CLASS budgetClass,
    std::size_t bytes,
    std::size_t alignment) {
    if (!owner.IsValid()) {
        return memory_accounting_result_t::Failure(MEMORY_ACCOUNTING_STATUS::InvalidOwner);
    }

    if (!tag.IsValid()) {
        return memory_accounting_result_t::Failure(MEMORY_ACCOUNTING_STATUS::InvalidTag);
    }

    if (bytes == 0U) {
        return memory_accounting_result_t::Failure(MEMORY_ACCOUNTING_STATUS::InvalidSize);
    }

    if (!IsPowerOfTwo(alignment)) {
        return memory_accounting_result_t::Failure(MEMORY_ACCOUNTING_STATUS::InvalidAlignment);
    }

    if (!IsValidMemoryBudgetClass(budgetClass)) {
        return memory_accounting_result_t::Failure(MEMORY_ACCOUNTING_STATUS::InvalidBudgetClass);
    }

    if (owner.Value.size() > MAX_MEMORY_OWNER_ID_BYTES) {
        return memory_accounting_result_t::Failure(MEMORY_ACCOUNTING_STATUS::InvalidOwner);
    }

    if (tag.Value.size() > MAX_MEMORY_TAG_BYTES) {
        return memory_accounting_result_t::Failure(MEMORY_ACCOUNTING_STATUS::InvalidTag);
    }

    if (IsHotMemoryBudgetClass(budgetClass)) {
        return memory_accounting_result_t::Failure(MEMORY_ACCOUNTING_STATUS::BudgetExceeded);
    }

    active_allocation_record_t* record = FindFreeAllocationRecord();
    if (record == nullptr) {
        return memory_accounting_result_t::Failure(MEMORY_ACCOUNTING_STATUS::CapacityExceeded);
    }

    const memory_allocation_id_t allocationId{_nextAllocationId};
    ++_nextAllocationId;

    if (!CopyFixedText(owner.Value, record->Owner, record->OwnerLength)) {
        return memory_accounting_result_t::Failure(MEMORY_ACCOUNTING_STATUS::InvalidOwner);
    }

    if (!CopyFixedText(tag.Value, record->Tag, record->TagLength)) {
        ResetAllocationRecord(*record);
        return memory_accounting_result_t::Failure(MEMORY_ACCOUNTING_STATUS::InvalidTag);
    }

    record->IsActive = true;
    record->AllocationId = allocationId;
    record->Bytes = bytes;
    ++_activeAllocationCount;

    ++_snapshot.AllocationCount;
    _snapshot.RetainedBytes += bytes;
    if (_snapshot.RetainedBytes > _snapshot.PeakRetainedBytes) {
        _snapshot.PeakRetainedBytes = _snapshot.RetainedBytes;
    }

    _snapshot.LeakCount = _activeAllocationCount;
    ++_budgetAllocationCounts[MemoryBudgetClassIndex(budgetClass)];
    return memory_accounting_result_t::Success(allocationId);
}

MEMORY_ACCOUNTING_STATUS CountingMemoryTracker::RecordFree(memory_allocation_id_t allocationId, memory_owner_id_t owner, memory_tag_t tag) {
    active_allocation_record_t* record = FindActiveAllocation(allocationId);
    if (record == nullptr) {
        return MEMORY_ACCOUNTING_STATUS::UnmatchedFree;
    }

    if (!FixedTextEquals(record->Owner, record->OwnerLength, owner.Value)) {
        return MEMORY_ACCOUNTING_STATUS::OwnerTagMismatch;
    }

    if (!FixedTextEquals(record->Tag, record->TagLength, tag.Value)) {
        return MEMORY_ACCOUNTING_STATUS::OwnerTagMismatch;
    }

    const std::size_t bytes = record->Bytes;
    if (bytes > _snapshot.RetainedBytes) {
        return MEMORY_ACCOUNTING_STATUS::UnmatchedFree;
    }

    _snapshot.RetainedBytes -= bytes;
    ++_snapshot.FreeCount;

    ResetAllocationRecord(*record);
    --_activeAllocationCount;
    _snapshot.LeakCount = _activeAllocationCount;
    return MEMORY_ACCOUNTING_STATUS::Success;
}

memory_snapshot_t CountingMemoryTracker::Snapshot() const {
    return _snapshot;
}

std::uint64_t CountingMemoryTracker::AllocationCountForBudget(MEMORY_BUDGET_CLASS budgetClass) const {
    if (!IsValidMemoryBudgetClass(budgetClass)) {
        return 0U;
    }

    return _budgetAllocationCounts[MemoryBudgetClassIndex(budgetClass)];
}

active_allocation_record_t* CountingMemoryTracker::FindActiveAllocation(memory_allocation_id_t allocationId) {
    for (active_allocation_record_t& record : _activeAllocations) {
        if (record.IsActive && record.AllocationId.Value == allocationId.Value) {
            return &record;
        }
    }

    return nullptr;
}

active_allocation_record_t* CountingMemoryTracker::FindFreeAllocationRecord() {
    for (active_allocation_record_t& record : _activeAllocations) {
        if (!record.IsActive) {
            return &record;
        }
    }

    return nullptr;
}

void CountingMemoryTracker::ResetAllocationRecord(active_allocation_record_t& record) {
    record = active_allocation_record_t{};
}
}
