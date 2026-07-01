// 模块: YuEngine Memory
// 文件: Src/YuEngine/Memory/Src/CountingMemoryTracker.cpp

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
void ClearFixedText(std::array<char, Capacity> &destination, std::size_t &length) {
    for (std::size_t index = 0U; index < Capacity; ++index) {
        destination[index] = 0;
    }

    length = 0U;
}

template <std::size_t Capacity>
void CopyFixedTextUnchecked(std::string_view source, std::array<char, Capacity> &destination, std::size_t &length) {
    ClearFixedText(destination, length);

    for (std::size_t index = 0U; index < source.size(); ++index) {
        destination[index] = source[index];
    }

    length = source.size();
}

template <std::size_t Capacity>
bool CopyFixedText(std::string_view source, std::array<char, Capacity> &destination, std::size_t &length) {
    if (source.size() > Capacity) {
        return false;
    }

    CopyFixedTextUnchecked(source, destination, length);
    return true;
}

template <std::size_t Capacity>
bool FixedTextEquals(const std::array<char, Capacity>& stored, std::size_t stored_length, std::string_view candidate) {
    if (candidate.size() != stored_length) {
        return false;
    }

    for (std::size_t index = 0U; index < stored_length; ++index) {
        if (stored[index] != candidate[index]) {
            return false;
        }
    }

    return true;
}

void ClearAllocationCapacityEntry(MemorySnapshot &snapshot) {
    snapshot.last_allocation_capacity_entry_requested_bytes = 0U;
    ClearFixedText(
        snapshot.last_allocation_capacity_entry_owner,
        snapshot.last_allocation_capacity_entry_owner_length);
    ClearFixedText(
        snapshot.last_allocation_capacity_entry_tag,
        snapshot.last_allocation_capacity_entry_tag_length);
    snapshot.last_allocation_capacity_entry_budget_class = MemoryBudgetClass::Setup;
    snapshot.last_allocation_capacity_entry_capacity = 0U;
    snapshot.last_allocation_capacity_entry_active_count = 0U;
    snapshot.last_allocation_capacity_entry_retained_bytes = 0U;
}

void StoreAllocationCapacityEntry(
    MemorySnapshot &snapshot,
    MemoryAccountingResult &result,
    MemoryOwnerId owner,
    MemoryTag tag,
    MemoryBudgetClass budget_class,
    std::size_t bytes,
    std::size_t active_allocation_count) {
    result.required_allocation_count = snapshot.required_allocation_count;
    result.capacity_entry_requested_bytes = bytes;
    CopyFixedTextUnchecked(owner.value, result.capacity_entry_owner, result.capacity_entry_owner_length);
    CopyFixedTextUnchecked(tag.value, result.capacity_entry_tag, result.capacity_entry_tag_length);
    result.capacity_entry_budget_class = budget_class;
    result.capacity_entry_allocation_capacity = snapshot.allocation_capacity;
    result.capacity_entry_active_allocation_count = active_allocation_count;
    result.capacity_entry_retained_bytes = snapshot.retained_bytes;

    snapshot.last_allocation_capacity_entry_requested_bytes = bytes;
    CopyFixedTextUnchecked(
        owner.value,
        snapshot.last_allocation_capacity_entry_owner,
        snapshot.last_allocation_capacity_entry_owner_length);
    CopyFixedTextUnchecked(
        tag.value,
        snapshot.last_allocation_capacity_entry_tag,
        snapshot.last_allocation_capacity_entry_tag_length);
    snapshot.last_allocation_capacity_entry_budget_class = budget_class;
    snapshot.last_allocation_capacity_entry_capacity = snapshot.allocation_capacity;
    snapshot.last_allocation_capacity_entry_active_count = active_allocation_count;
    snapshot.last_allocation_capacity_entry_retained_bytes = snapshot.retained_bytes;
}
}

CountingMemoryTracker::CountingMemoryTracker()
    : active_allocations_(),
      budget_allocation_counts_{},
      snapshot_(),
      next_allocation_id_(1U),
      active_allocation_count_(0U) {
    snapshot_.allocation_capacity = MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS;
    snapshot_.required_allocation_count = 1U;
}

MemoryAccountingResult CountingMemoryTracker::RecordAllocation(
    MemoryOwnerId owner,
    MemoryTag tag,
    MemoryBudgetClass budget_class,
    std::size_t bytes,
    std::size_t alignment) {
    if (!owner.IsValid()) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::InvalidOwner;
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidOwner);
    }

    if (!tag.IsValid()) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::InvalidTag;
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidTag);
    }

    if (bytes == 0U) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::InvalidSize;
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidSize);
    }

    if (!IsPowerOfTwo(alignment)) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::InvalidAlignment;
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidAlignment);
    }

    if (!IsValidMemoryBudgetClass(budget_class)) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::InvalidBudgetClass;
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidBudgetClass);
    }

    if (owner.value.size() > MAX_MEMORY_OWNER_ID_BYTES) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::InvalidOwner;
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidOwner);
    }

    if (tag.value.size() > MAX_MEMORY_TAG_BYTES) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::InvalidTag;
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidTag);
    }

    if (IsHotMemoryBudgetClass(budget_class)) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::BudgetExceeded;
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::BudgetExceeded);
    }

    snapshot_.required_allocation_count = active_allocation_count_ + 1U;
    ActiveAllocationRecord* record = FindFreeAllocationRecord();
    if (record == nullptr) {
        snapshot_.last_status = MemoryAccountingStatus::CapacityExceeded;
        MemoryAccountingResult result = MemoryAccountingResult::Failure(MemoryAccountingStatus::CapacityExceeded);
        StoreAllocationCapacityEntry(snapshot_, result, owner, tag, budget_class, bytes, active_allocation_count_);
        return result;
    }

    const MemoryAllocationId allocation_id{next_allocation_id_};
    ++next_allocation_id_;

    if (!CopyFixedText(owner.value, record->owner, record->owner_length)) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::InvalidOwner;
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidOwner);
    }

    if (!CopyFixedText(tag.value, record->tag, record->tag_length)) {
        ResetAllocationRecord(*record);
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::InvalidTag;
        return MemoryAccountingResult::Failure(MemoryAccountingStatus::InvalidTag);
    }

    record->is_active = true;
    record->allocation_id = allocation_id;
    record->bytes = bytes;
    ++active_allocation_count_;

    ++snapshot_.allocation_count;
    snapshot_.retained_bytes += bytes;
    if (snapshot_.retained_bytes > snapshot_.peak_retained_bytes) {
        snapshot_.peak_retained_bytes = snapshot_.retained_bytes;
    }

    snapshot_.leak_count = active_allocation_count_;
    ++budget_allocation_counts_[MemoryBudgetClassIndex(budget_class)];
    ClearAllocationCapacityEntry(snapshot_);
    snapshot_.last_status = MemoryAccountingStatus::Success;
    return MemoryAccountingResult::Success(allocation_id);
}

MemoryAccountingStatus CountingMemoryTracker::RecordFree(MemoryAllocationId allocation_id, MemoryOwnerId owner, MemoryTag tag) {
    ActiveAllocationRecord* record = FindActiveAllocation(allocation_id);
    if (record == nullptr) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::UnmatchedFree;
        return MemoryAccountingStatus::UnmatchedFree;
    }

    if (!FixedTextEquals(record->owner, record->owner_length, owner.value)) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::OwnerTagMismatch;
        return MemoryAccountingStatus::OwnerTagMismatch;
    }

    if (!FixedTextEquals(record->tag, record->tag_length, tag.value)) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::OwnerTagMismatch;
        return MemoryAccountingStatus::OwnerTagMismatch;
    }

    const std::size_t bytes = record->bytes;
    if (bytes > snapshot_.retained_bytes) {
        ClearAllocationCapacityEntry(snapshot_);
        snapshot_.last_status = MemoryAccountingStatus::UnmatchedFree;
        return MemoryAccountingStatus::UnmatchedFree;
    }

    snapshot_.retained_bytes -= bytes;
    ++snapshot_.free_count;

    ResetAllocationRecord(*record);
    --active_allocation_count_;
    snapshot_.leak_count = active_allocation_count_;
    ClearAllocationCapacityEntry(snapshot_);
    snapshot_.last_status = MemoryAccountingStatus::Success;
    return MemoryAccountingStatus::Success;
}

MemorySnapshot CountingMemoryTracker::Snapshot() const {
    return snapshot_;
}

std::uint64_t CountingMemoryTracker::AllocationCountForBudget(MemoryBudgetClass budget_class) const {
    if (!IsValidMemoryBudgetClass(budget_class)) {
        return 0U;
    }

    return budget_allocation_counts_[MemoryBudgetClassIndex(budget_class)];
}

ActiveAllocationRecord* CountingMemoryTracker::FindActiveAllocation(MemoryAllocationId allocation_id) {
    for (ActiveAllocationRecord& record : active_allocations_) {
        if (record.is_active && record.allocation_id.value == allocation_id.value) {
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
