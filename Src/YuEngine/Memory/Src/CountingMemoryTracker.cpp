#include "YuEngine/Memory/CountingMemoryTracker.h"

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
    : active_allocations_(),
      budget_allocation_counts_{},
      snapshot_{0U, 0U, 0U, 0U, 0U},
      next_allocation_id_(1U),
      active_allocation_count_(0U) {
}

MemoryAccountingResult CountingMemoryTracker::RecordAllocation(
    MemoryOwnerId owner,
    MemoryTag tag,
    MemoryBudgetClass budgetClass,
    std::size_t bytes,
    std::size_t alignment) {
    if (!owner.IsValid()) {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidOwner);
    }

    if (!tag.IsValid()) {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidTag);
    }

    if (bytes == 0U) {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidSize);
    }

    if (!IsPowerOfTwo(alignment)) {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidAlignment);
    }

    if (!IsValidMemoryBudgetClass(budgetClass)) {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidBudgetClass);
    }

    if (owner.value.size() > MAX_MEMORY_OWNER_ID_BYTES) {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidOwner);
    }

    if (tag.value.size() > MAX_MEMORY_TAG_BYTES) {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidTag);
    }

    if (IsHotMemoryBudgetClass(budgetClass)) {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::BudgetExceeded);
    }

    ActiveAllocationRecord* record = FindFreeAllocationRecord();
    if (record == nullptr) {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::CapacityExceeded);
    }

    const MemoryAllocationId allocationId{next_allocation_id_};
    ++next_allocation_id_;

    if (!CopyFixedText(owner.value, record->owner, record->owner_length)) {
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidOwner);
    }

    if (!CopyFixedText(tag.value, record->tag, record->tag_length)) {
        ResetAllocationRecord(*record);
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidTag);
    }

    record->is_active = true;
    record->allocation_id = allocationId;
    record->bytes = bytes;
    ++active_allocation_count_;

    ++snapshot_.allocation_count;
    snapshot_.retained_bytes += bytes;
    if (snapshot_.retained_bytes > snapshot_.peak_retained_bytes) {
        snapshot_.peak_retained_bytes = snapshot_.retained_bytes;
    }

    snapshot_.leak_count = active_allocation_count_;
    ++budget_allocation_counts_[MemoryBudgetClassIndex(budgetClass)];
    return MemoryAccountingResult::Success(allocationId);
}

MemoryAccountingStatus CountingMemoryTracker::RecordFree(MemoryAllocationId allocationId, MemoryOwnerId owner, MemoryTag tag) {
    ActiveAllocationRecord* record = FindActiveAllocation(allocationId);
    if (record == nullptr) {
        return MemoryAccountingStatus::UnmatchedFree;
    }

    if (!FixedTextEquals(record->owner, record->owner_length, owner.value)) {
        return MemoryAccountingStatus::OwnerTagMismatch;
    }

    if (!FixedTextEquals(record->tag, record->tag_length, tag.value)) {
        return MemoryAccountingStatus::OwnerTagMismatch;
    }

    const std::size_t bytes = record->bytes;
    if (bytes > snapshot_.retained_bytes) {
        return MemoryAccountingStatus::UnmatchedFree;
    }

    snapshot_.retained_bytes -= bytes;
    ++snapshot_.free_count;

    ResetAllocationRecord(*record);
    --active_allocation_count_;
    snapshot_.leak_count = active_allocation_count_;
    return MemoryAccountingStatus::Success;
}

MemorySnapshot CountingMemoryTracker::Snapshot() const {
    return snapshot_;
}

std::uint64_t CountingMemoryTracker::AllocationCountForBudget(MemoryBudgetClass budgetClass) const {
    if (!IsValidMemoryBudgetClass(budgetClass)) {
        return 0U;
    }

    return budget_allocation_counts_[MemoryBudgetClassIndex(budgetClass)];
}

ActiveAllocationRecord* CountingMemoryTracker::FindActiveAllocation(MemoryAllocationId allocationId) {
    for (ActiveAllocationRecord& record : active_allocations_) {
        if (record.is_active && record.allocation_id.value == allocationId.value) {
            return &record;
        }
    }

    return nullptr;
}

ActiveAllocationRecord* CountingMemoryTracker::FindFreeAllocationRecord() {
    for (ActiveAllocationRecord& record : active_allocations_) {
        if (!record.is_active) {
            return &record;
        }
    }

    return nullptr;
}

void CountingMemoryTracker::ResetAllocationRecord(ActiveAllocationRecord& record) {
    record = ActiveAllocationRecord{};
}
}
