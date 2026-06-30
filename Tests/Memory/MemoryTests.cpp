// 模块：Tests Memory
// 文件：Tests/Memory/MemoryTests.cpp

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/Memory/CountingMemoryTracker.h"
#include "YuEngine/Memory/DisabledMemoryTracker.h"

using CountingMemoryTracker = yuengine::memory::CountingMemoryTracker;
using DisabledMemoryTracker = yuengine::memory::DisabledMemoryTracker;
using yuengine::memory::MemoryAccountingStatus;
using yuengine::memory::MemoryBudgetClass;
using yuengine::memory::MemoryOwnerId;
using yuengine::memory::MemoryTag;
using yuengine::memory::MemoryAllocationId;
using yuengine::memory::MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS;

namespace {
constexpr const char* TEST_COUNTS = "Memory_TrackerCountsAllocationAndFree";
constexpr const char* TEST_PEAK = "Memory_TrackerReportsPeakAndRetainedBytes";
constexpr const char* TEST_LEAK = "Memory_TrackerReportsLeakOnUnreleasedBytes";
constexpr const char* TEST_UNMATCHED_FREE = "Memory_TrackerRejectsUnmatchedFree";
constexpr const char* TEST_BUDGET_CLASS = "Memory_TrackerRecordsBudgetClass";
constexpr const char* TEST_DISABLED = "Memory_DisabledTracker_DoesNotChangeBehavior";
constexpr const char* TEST_HOT_PATH = "Memory_HotPathBudget_FailsOnTrackedAllocation";
constexpr const char* TEST_FIXED_CAPACITY = "Memory_TrackerRejectsBeyondFixedCapacityWithoutMutation";
constexpr const char* TEST_OWNER_TAG_BYTE_CAPS = "Memory_TrackerEnforcesOwnerAndTagByteCapsWithoutMutation";
constexpr const char *TEST_LAST_STATUS = "Memory_TrackerRecordsLastStatusForAllocationAndFree";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char* OWNER_PLATFORM = "Platform";
constexpr const char* OWNER_KERNEL = "Kernel";
constexpr const char* TAG_FIXTURE = "Fixture";
constexpr std::size_t SMALL_BYTES = 16U;
constexpr std::size_t MEDIUM_BYTES = 48U;
constexpr std::size_t LARGE_BYTES = 64U;
constexpr std::size_t ALIGNMENT = 8U;
constexpr std::size_t TRACKED_TEXT_CAP_BYTES = 64U;
constexpr std::size_t TRACKED_TEXT_OVER_CAP_BYTES = TRACKED_TEXT_CAP_BYTES + 1U;
using TestFunction = int (*)();

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int MemoryTrackerCountsAllocationAndFree() {
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};

    const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, LARGE_BYTES, ALIGNMENT);
    if (!allocation.Succeeded()) {
        return Fail("tracked setup allocation failed");
    }

    const auto after_allocation = tracker.Snapshot();
    if (after_allocation.allocation_count != 1U) {
        return Fail("allocation count did not increment");
    }

    if (after_allocation.retained_bytes != LARGE_BYTES) {
        return Fail("retained bytes did not match allocation");
    }

    const auto free_status = tracker.RecordFree(allocation.allocation_id, owner, tag);
    if (free_status != MemoryAccountingStatus::Success) {
        return Fail("tracked free failed");
    }

    const auto after_free = tracker.Snapshot();
    if (after_free.free_count != 1U) {
        return Fail("free count did not increment");
    }

    if (after_free.retained_bytes != 0U) {
        return Fail("retained bytes did not return to zero");
    }

    if (after_free.HasLeaks()) {
        return Fail("freed fixture still reported leaks");
    }

    return 0;
}

int MemoryTrackerReportsPeakAndRetainedBytes() {
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};

    const auto first_allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, SMALL_BYTES, ALIGNMENT);
    const auto second_allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Load, MEDIUM_BYTES, ALIGNMENT);
    if (!first_allocation.Succeeded()) {
        return Fail("first allocation failed");
    }

    if (!second_allocation.Succeeded()) {
        return Fail("second allocation failed");
    }

    const auto peak_snapshot = tracker.Snapshot();
    if (peak_snapshot.peak_retained_bytes != SMALL_BYTES + MEDIUM_BYTES) {
        return Fail("peak retained bytes did not record high watermark");
    }

    const auto free_status = tracker.RecordFree(first_allocation.allocation_id, owner, tag);
    if (free_status != MemoryAccountingStatus::Success) {
        return Fail("first free failed");
    }

    const auto retained_snapshot = tracker.Snapshot();
    if (retained_snapshot.retained_bytes != MEDIUM_BYTES) {
        return Fail("retained bytes after partial free were wrong");
    }

    if (retained_snapshot.peak_retained_bytes != SMALL_BYTES + MEDIUM_BYTES) {
        return Fail("peak retained bytes changed after free");
    }

    const auto cleanup_status = tracker.RecordFree(second_allocation.allocation_id, owner, tag);
    if (cleanup_status != MemoryAccountingStatus::Success) {
        return Fail("cleanup free failed");
    }

    return 0;
}

int MemoryTrackerReportsLeakOnUnreleasedBytes() {
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};

    const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Load, MEDIUM_BYTES, ALIGNMENT);
    if (!allocation.Succeeded()) {
        return Fail("leak fixture allocation failed");
    }

    const auto snapshot = tracker.Snapshot();
    if (!snapshot.HasLeaks()) {
        return Fail("unreleased allocation was not reported as leak");
    }

    if (snapshot.leak_count != 1U) {
        return Fail("leak count was wrong");
    }

    if (snapshot.retained_bytes != MEDIUM_BYTES) {
        return Fail("leak retained bytes were wrong");
    }

    return 0;
}

int MemoryTrackerRejectsUnmatchedFree() {
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryOwnerId other_owner{OWNER_KERNEL};
    const MemoryTag tag{TAG_FIXTURE};

    const auto unmatched_status = tracker.RecordFree(MemoryAllocationId{999U}, owner, tag);
    if (unmatched_status != MemoryAccountingStatus::UnmatchedFree) {
        return Fail("unmatched free was not rejected");
    }

    const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, SMALL_BYTES, ALIGNMENT);
    if (!allocation.Succeeded()) {
        return Fail("owner mismatch fixture allocation failed");
    }

    const auto mismatch_status = tracker.RecordFree(allocation.allocation_id, other_owner, tag);
    if (mismatch_status != MemoryAccountingStatus::OwnerTagMismatch) {
        return Fail("owner mismatch was not rejected");
    }

    const auto snapshot = tracker.Snapshot();
    if (snapshot.retained_bytes != SMALL_BYTES) {
        return Fail("owner mismatch changed retained bytes");
    }

    const auto cleanup_status = tracker.RecordFree(allocation.allocation_id, owner, tag);
    if (cleanup_status != MemoryAccountingStatus::Success) {
        return Fail("cleanup after owner mismatch failed");
    }

    return 0;
}

int MemoryTrackerRecordsBudgetClass() {
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};

    const auto setup_allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, SMALL_BYTES, ALIGNMENT);
    const auto load_allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Load, MEDIUM_BYTES, ALIGNMENT);
    if (!setup_allocation.Succeeded()) {
        return Fail("setup budget allocation failed");
    }

    if (!load_allocation.Succeeded()) {
        return Fail("load budget allocation failed");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Setup) != 1U) {
        return Fail("setup budget allocation count was wrong");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Load) != 1U) {
        return Fail("load budget allocation count was wrong");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Tool) != 0U) {
        return Fail("unallocated budget class reported allocations");
    }

    const auto setup_free = tracker.RecordFree(setup_allocation.allocation_id, owner, tag);
    const auto load_free = tracker.RecordFree(load_allocation.allocation_id, owner, tag);
    if (setup_free != MemoryAccountingStatus::Success) {
        return Fail("setup budget cleanup failed");
    }

    if (load_free != MemoryAccountingStatus::Success) {
        return Fail("load budget cleanup failed");
    }

    return 0;
}

int MemoryDisabledTrackerDoesNotChangeBehavior() {
    DisabledMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};

    const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Frame, LARGE_BYTES, ALIGNMENT);
    if (!allocation.Succeeded()) {
        return Fail("disabled tracker changed allocation behavior");
    }

    const auto free_status = tracker.RecordFree(allocation.allocation_id, owner, tag);
    if (free_status != MemoryAccountingStatus::Success) {
        return Fail("disabled tracker changed free behavior");
    }

    const auto snapshot = tracker.Snapshot();
    if (snapshot.allocation_count != 0U) {
        return Fail("disabled tracker counted allocations");
    }

    if (snapshot.retained_bytes != 0U) {
        return Fail("disabled tracker retained bytes");
    }

    if (snapshot.last_status != MemoryAccountingStatus::Success) {
        return Fail("disabled tracker last status changed");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Frame) != 0U) {
        return Fail("disabled tracker counted hot-path budget allocations");
    }

    return 0;
}

int MemoryHotPathBudgetFailsOnTrackedAllocation() {
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};

    const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Frame, SMALL_BYTES, ALIGNMENT);
    if (allocation.status != MemoryAccountingStatus::BudgetExceeded) {
        return Fail("hot-path tracked allocation did not fail zero budget");
    }

    const auto snapshot = tracker.Snapshot();
    if (snapshot.allocation_count != 0U) {
        return Fail("failed hot-path allocation changed allocation count");
    }

    if (snapshot.retained_bytes != 0U) {
        return Fail("failed hot-path allocation changed retained bytes");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Frame) != 0U) {
        return Fail("failed hot-path allocation changed budget count");
    }

    return 0;
}

int MemoryTrackerRejectsBeyondFixedCapacityWithoutMutation() {
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};
    std::array<MemoryAllocationId, MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS> allocations{};

    for (std::size_t index = 0U; index < allocations.size(); ++index) {
        const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, SMALL_BYTES, ALIGNMENT);
        if (!allocation.Succeeded()) {
            return Fail("fixed-capacity setup allocation failed before capacity");
        }

        allocations[index] = allocation.allocation_id;
    }

    const auto before_overflow = tracker.Snapshot();
    const auto overflow = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, SMALL_BYTES, ALIGNMENT);
    if (overflow.status != MemoryAccountingStatus::CapacityExceeded) {
        return Fail("fixed-capacity tracker did not reject allocation overflow");
    }

    if (overflow.allocation_id.value != 0U) {
        return Fail("failed capacity allocation returned an allocation id");
    }

    const auto after_overflow = tracker.Snapshot();
    if (after_overflow.allocation_count != before_overflow.allocation_count) {
        return Fail("capacity overflow changed allocation count");
    }

    if (after_overflow.retained_bytes != before_overflow.retained_bytes) {
        return Fail("capacity overflow changed retained bytes");
    }

    if (after_overflow.leak_count != before_overflow.leak_count) {
        return Fail("capacity overflow changed active leak count");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Setup) != allocations.size()) {
        return Fail("capacity overflow changed setup budget count");
    }

    for (const MemoryAllocationId allocation_id : allocations) {
        if (tracker.RecordFree(allocation_id, owner, tag) != MemoryAccountingStatus::Success) {
            return Fail("fixed-capacity cleanup free failed");
        }
    }

    const auto after_cleanup = tracker.Snapshot();
    if (after_cleanup.HasLeaks()) {
        return Fail("fixed-capacity cleanup left leaks");
    }

    return 0;
}

int MemoryTrackerEnforcesOwnerAndTagByteCapsWithoutMutation() {
    CountingMemoryTracker tracker;
    const std::string max_owner(TRACKED_TEXT_CAP_BYTES, 'O');
    const std::string max_tag(TRACKED_TEXT_CAP_BYTES, 'T');
    const std::string oversized_owner(TRACKED_TEXT_OVER_CAP_BYTES, 'O');
    const std::string oversized_tag(TRACKED_TEXT_OVER_CAP_BYTES, 'T');

    const auto accepted = tracker.RecordAllocation(
        MemoryOwnerId{max_owner},
        MemoryTag{max_tag},
        MemoryBudgetClass::Setup,
        SMALL_BYTES,
        ALIGNMENT);
    if (!accepted.Succeeded()) {
        return Fail("max byte owner/tag allocation failed");
    }

    if (tracker.RecordFree(accepted.allocation_id, MemoryOwnerId{max_owner}, MemoryTag{max_tag}) !=
        MemoryAccountingStatus::Success) {
        return Fail("max byte owner/tag cleanup failed");
    }

    const auto before_rejects = tracker.Snapshot();
    const auto owner_reject = tracker.RecordAllocation(
        MemoryOwnerId{oversized_owner},
        MemoryTag{max_tag},
        MemoryBudgetClass::Setup,
        SMALL_BYTES,
        ALIGNMENT);
    if (owner_reject.status != MemoryAccountingStatus::InvalidOwner) {
        return Fail("oversized owner was not rejected");
    }

    const auto tag_reject = tracker.RecordAllocation(
        MemoryOwnerId{max_owner},
        MemoryTag{oversized_tag},
        MemoryBudgetClass::Setup,
        SMALL_BYTES,
        ALIGNMENT);
    if (tag_reject.status != MemoryAccountingStatus::InvalidTag) {
        return Fail("oversized tag was not rejected");
    }

    const auto after_rejects = tracker.Snapshot();
    if (after_rejects.allocation_count != before_rejects.allocation_count) {
        return Fail("oversized owner/tag reject changed allocation count");
    }

    if (after_rejects.free_count != before_rejects.free_count) {
        return Fail("oversized owner/tag reject changed free count");
    }

    if (after_rejects.retained_bytes != before_rejects.retained_bytes) {
        return Fail("oversized owner/tag reject changed retained bytes");
    }

    if (after_rejects.peak_retained_bytes != before_rejects.peak_retained_bytes) {
        return Fail("oversized owner/tag reject changed peak bytes");
    }

    if (after_rejects.leak_count != before_rejects.leak_count) {
        return Fail("oversized owner/tag reject changed leak count");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Setup) != 1U) {
        return Fail("oversized owner/tag reject changed setup budget count");
    }

    return 0;
}

int MemoryTrackerRecordsLastStatusForAllocationAndFree() {
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryOwnerId other_owner{OWNER_KERNEL};
    const MemoryTag tag{TAG_FIXTURE};

    const auto initial_snapshot = tracker.Snapshot();
    if (initial_snapshot.last_status != MemoryAccountingStatus::Success) {
        return Fail("initial memory last status was not success");
    }

    const auto invalid_owner = tracker.RecordAllocation(
        MemoryOwnerId{},
        tag,
        MemoryBudgetClass::Setup,
        SMALL_BYTES,
        ALIGNMENT);
    if (invalid_owner.status != MemoryAccountingStatus::InvalidOwner) {
        return Fail("invalid owner allocation status wrong");
    }

    const auto after_invalid_owner = tracker.Snapshot();
    if (after_invalid_owner.last_status != MemoryAccountingStatus::InvalidOwner) {
        return Fail("invalid owner did not update last status");
    }

    const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, SMALL_BYTES, ALIGNMENT);
    if (!allocation.Succeeded()) {
        return Fail("last status fixture allocation failed");
    }

    const auto after_allocation = tracker.Snapshot();
    if (after_allocation.last_status != MemoryAccountingStatus::Success) {
        return Fail("allocation success did not reset last status");
    }

    const auto mismatch_status = tracker.RecordFree(allocation.allocation_id, other_owner, tag);
    if (mismatch_status != MemoryAccountingStatus::OwnerTagMismatch) {
        return Fail("last status owner mismatch status wrong");
    }

    const auto after_mismatch = tracker.Snapshot();
    if (after_mismatch.last_status != MemoryAccountingStatus::OwnerTagMismatch) {
        return Fail("owner mismatch did not update last status");
    }

    const auto cleanup_status = tracker.RecordFree(allocation.allocation_id, owner, tag);
    if (cleanup_status != MemoryAccountingStatus::Success) {
        return Fail("last status cleanup free failed");
    }

    const auto after_cleanup = tracker.Snapshot();
    if (after_cleanup.last_status != MemoryAccountingStatus::Success) {
        return Fail("free success did not reset last status");
    }

    const auto unmatched_status = tracker.RecordFree(MemoryAllocationId{999U}, owner, tag);
    if (unmatched_status != MemoryAccountingStatus::UnmatchedFree) {
        return Fail("last status unmatched free status wrong");
    }

    const auto after_unmatched = tracker.Snapshot();
    if (after_unmatched.last_status != MemoryAccountingStatus::UnmatchedFree) {
        return Fail("unmatched free did not update last status");
    }

    if (after_unmatched.allocation_count != after_cleanup.allocation_count) {
        return Fail("last status unmatched free changed allocation count");
    }

    if (after_unmatched.free_count != after_cleanup.free_count) {
        return Fail("last status unmatched free changed free count");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_COUNTS, MemoryTrackerCountsAllocationAndFree},
        {TEST_PEAK, MemoryTrackerReportsPeakAndRetainedBytes},
        {TEST_LEAK, MemoryTrackerReportsLeakOnUnreleasedBytes},
        {TEST_UNMATCHED_FREE, MemoryTrackerRejectsUnmatchedFree},
        {TEST_BUDGET_CLASS, MemoryTrackerRecordsBudgetClass},
        {TEST_DISABLED, MemoryDisabledTrackerDoesNotChangeBehavior},
        {TEST_HOT_PATH, MemoryHotPathBudgetFailsOnTrackedAllocation},
        {TEST_FIXED_CAPACITY, MemoryTrackerRejectsBeyondFixedCapacityWithoutMutation},
        {TEST_OWNER_TAG_BYTE_CAPS, MemoryTrackerEnforcesOwnerAndTagByteCapsWithoutMutation},
        {TEST_LAST_STATUS, MemoryTrackerRecordsLastStatusForAllocationAndFree}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
