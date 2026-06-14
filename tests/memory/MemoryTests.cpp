#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>

#include "yuengine/memory/CountingMemoryTracker.h"
#include "yuengine/memory/DisabledMemoryTracker.h"

using CountingMemoryTracker = yuengine::memory::CountingMemoryTracker;
using DisabledMemoryTracker = yuengine::memory::DisabledMemoryTracker;
using MemoryAccountingStatus = yuengine::memory::MemoryAccountingStatus;
using MemoryBudgetClass = yuengine::memory::MemoryBudgetClass;
using MemoryOwnerId = yuengine::memory::MemoryOwnerId;
using MemoryTag = yuengine::memory::MemoryTag;
using MemoryAllocationId = yuengine::memory::MemoryAllocationId;
using yuengine::memory::MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS;

namespace
{
constexpr const char* TEST_COUNTS = "Memory_TrackerCountsAllocationAndFree";
constexpr const char* TEST_PEAK = "Memory_TrackerReportsPeakAndRetainedBytes";
constexpr const char* TEST_LEAK = "Memory_TrackerReportsLeakOnUnreleasedBytes";
constexpr const char* TEST_UNMATCHED_FREE = "Memory_TrackerRejectsUnmatchedFree";
constexpr const char* TEST_BUDGET_CLASS = "Memory_TrackerRecordsBudgetClass";
constexpr const char* TEST_DISABLED = "Memory_DisabledTracker_DoesNotChangeBehavior";
constexpr const char* TEST_HOT_PATH = "Memory_HotPathBudget_FailsOnTrackedAllocation";
constexpr const char* TEST_FIXED_CAPACITY = "Memory_TrackerRejectsBeyondFixedCapacityWithoutMutation";
constexpr const char* TEST_OWNER_TAG_BYTE_CAPS = "Memory_TrackerEnforcesOwnerAndTagByteCapsWithoutMutation";
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

int Fail(const std::string& message)
{
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int MemoryTrackerCountsAllocationAndFree()
{
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};

    const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, LARGE_BYTES, ALIGNMENT);
    if (!allocation.Succeeded())
    {
        return Fail("tracked setup allocation failed");
    }

    const auto afterAllocation = tracker.Snapshot();
    if (afterAllocation.AllocationCount != 1U)
    {
        return Fail("allocation count did not increment");
    }

    if (afterAllocation.RetainedBytes != LARGE_BYTES)
    {
        return Fail("retained bytes did not match allocation");
    }

    const auto freeStatus = tracker.RecordFree(allocation.AllocationId, owner, tag);
    if (freeStatus != MemoryAccountingStatus::Success)
    {
        return Fail("tracked free failed");
    }

    const auto afterFree = tracker.Snapshot();
    if (afterFree.FreeCount != 1U)
    {
        return Fail("free count did not increment");
    }

    if (afterFree.RetainedBytes != 0U)
    {
        return Fail("retained bytes did not return to zero");
    }

    if (afterFree.HasLeaks())
    {
        return Fail("freed fixture still reported leaks");
    }

    return 0;
}

int MemoryTrackerReportsPeakAndRetainedBytes()
{
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};

    const auto firstAllocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, SMALL_BYTES, ALIGNMENT);
    const auto secondAllocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Load, MEDIUM_BYTES, ALIGNMENT);
    if (!firstAllocation.Succeeded())
    {
        return Fail("first allocation failed");
    }

    if (!secondAllocation.Succeeded())
    {
        return Fail("second allocation failed");
    }

    const auto peakSnapshot = tracker.Snapshot();
    if (peakSnapshot.PeakRetainedBytes != SMALL_BYTES + MEDIUM_BYTES)
    {
        return Fail("peak retained bytes did not record high watermark");
    }

    const auto freeStatus = tracker.RecordFree(firstAllocation.AllocationId, owner, tag);
    if (freeStatus != MemoryAccountingStatus::Success)
    {
        return Fail("first free failed");
    }

    const auto retainedSnapshot = tracker.Snapshot();
    if (retainedSnapshot.RetainedBytes != MEDIUM_BYTES)
    {
        return Fail("retained bytes after partial free were wrong");
    }

    if (retainedSnapshot.PeakRetainedBytes != SMALL_BYTES + MEDIUM_BYTES)
    {
        return Fail("peak retained bytes changed after free");
    }

    const auto cleanupStatus = tracker.RecordFree(secondAllocation.AllocationId, owner, tag);
    if (cleanupStatus != MemoryAccountingStatus::Success)
    {
        return Fail("cleanup free failed");
    }

    return 0;
}

int MemoryTrackerReportsLeakOnUnreleasedBytes()
{
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};

    const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Load, MEDIUM_BYTES, ALIGNMENT);
    if (!allocation.Succeeded())
    {
        return Fail("leak fixture allocation failed");
    }

    const auto snapshot = tracker.Snapshot();
    if (!snapshot.HasLeaks())
    {
        return Fail("unreleased allocation was not reported as leak");
    }

    if (snapshot.LeakCount != 1U)
    {
        return Fail("leak count was wrong");
    }

    if (snapshot.RetainedBytes != MEDIUM_BYTES)
    {
        return Fail("leak retained bytes were wrong");
    }

    return 0;
}

int MemoryTrackerRejectsUnmatchedFree()
{
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryOwnerId otherOwner{OWNER_KERNEL};
    const MemoryTag tag{TAG_FIXTURE};

    const auto unmatchedStatus = tracker.RecordFree(MemoryAllocationId{999U}, owner, tag);
    if (unmatchedStatus != MemoryAccountingStatus::UnmatchedFree)
    {
        return Fail("unmatched free was not rejected");
    }

    const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, SMALL_BYTES, ALIGNMENT);
    if (!allocation.Succeeded())
    {
        return Fail("owner mismatch fixture allocation failed");
    }

    const auto mismatchStatus = tracker.RecordFree(allocation.AllocationId, otherOwner, tag);
    if (mismatchStatus != MemoryAccountingStatus::OwnerTagMismatch)
    {
        return Fail("owner mismatch was not rejected");
    }

    const auto snapshot = tracker.Snapshot();
    if (snapshot.RetainedBytes != SMALL_BYTES)
    {
        return Fail("owner mismatch changed retained bytes");
    }

    const auto cleanupStatus = tracker.RecordFree(allocation.AllocationId, owner, tag);
    if (cleanupStatus != MemoryAccountingStatus::Success)
    {
        return Fail("cleanup after owner mismatch failed");
    }

    return 0;
}

int MemoryTrackerRecordsBudgetClass()
{
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};

    const auto setupAllocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, SMALL_BYTES, ALIGNMENT);
    const auto loadAllocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Load, MEDIUM_BYTES, ALIGNMENT);
    if (!setupAllocation.Succeeded())
    {
        return Fail("setup budget allocation failed");
    }

    if (!loadAllocation.Succeeded())
    {
        return Fail("load budget allocation failed");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Setup) != 1U)
    {
        return Fail("setup budget allocation count was wrong");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Load) != 1U)
    {
        return Fail("load budget allocation count was wrong");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Tool) != 0U)
    {
        return Fail("unallocated budget class reported allocations");
    }

    const auto setupFree = tracker.RecordFree(setupAllocation.AllocationId, owner, tag);
    const auto loadFree = tracker.RecordFree(loadAllocation.AllocationId, owner, tag);
    if (setupFree != MemoryAccountingStatus::Success)
    {
        return Fail("setup budget cleanup failed");
    }

    if (loadFree != MemoryAccountingStatus::Success)
    {
        return Fail("load budget cleanup failed");
    }

    return 0;
}

int MemoryDisabledTrackerDoesNotChangeBehavior()
{
    DisabledMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};

    const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Frame, LARGE_BYTES, ALIGNMENT);
    if (!allocation.Succeeded())
    {
        return Fail("disabled tracker changed allocation behavior");
    }

    const auto freeStatus = tracker.RecordFree(allocation.AllocationId, owner, tag);
    if (freeStatus != MemoryAccountingStatus::Success)
    {
        return Fail("disabled tracker changed free behavior");
    }

    const auto snapshot = tracker.Snapshot();
    if (snapshot.AllocationCount != 0U)
    {
        return Fail("disabled tracker counted allocations");
    }

    if (snapshot.RetainedBytes != 0U)
    {
        return Fail("disabled tracker retained bytes");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Frame) != 0U)
    {
        return Fail("disabled tracker counted hot-path budget allocations");
    }

    return 0;
}

int MemoryHotPathBudgetFailsOnTrackedAllocation()
{
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};

    const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Frame, SMALL_BYTES, ALIGNMENT);
    if (allocation.Status != MemoryAccountingStatus::BudgetExceeded)
    {
        return Fail("hot-path tracked allocation did not fail zero budget");
    }

    const auto snapshot = tracker.Snapshot();
    if (snapshot.AllocationCount != 0U)
    {
        return Fail("failed hot-path allocation changed allocation count");
    }

    if (snapshot.RetainedBytes != 0U)
    {
        return Fail("failed hot-path allocation changed retained bytes");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Frame) != 0U)
    {
        return Fail("failed hot-path allocation changed budget count");
    }

    return 0;
}

int MemoryTrackerRejectsBeyondFixedCapacityWithoutMutation()
{
    CountingMemoryTracker tracker;
    const MemoryOwnerId owner{OWNER_PLATFORM};
    const MemoryTag tag{TAG_FIXTURE};
    std::array<MemoryAllocationId, MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS> allocations{};

    for (std::size_t index = 0U; index < allocations.size(); ++index)
    {
        const auto allocation = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, SMALL_BYTES, ALIGNMENT);
        if (!allocation.Succeeded())
        {
            return Fail("fixed-capacity setup allocation failed before capacity");
        }

        allocations[index] = allocation.AllocationId;
    }

    const auto beforeOverflow = tracker.Snapshot();
    const auto overflow = tracker.RecordAllocation(owner, tag, MemoryBudgetClass::Setup, SMALL_BYTES, ALIGNMENT);
    if (overflow.Status != MemoryAccountingStatus::CapacityExceeded)
    {
        return Fail("fixed-capacity tracker did not reject allocation overflow");
    }

    if (overflow.AllocationId.Value != 0U)
    {
        return Fail("failed capacity allocation returned an allocation id");
    }

    const auto afterOverflow = tracker.Snapshot();
    if (afterOverflow.AllocationCount != beforeOverflow.AllocationCount)
    {
        return Fail("capacity overflow changed allocation count");
    }

    if (afterOverflow.RetainedBytes != beforeOverflow.RetainedBytes)
    {
        return Fail("capacity overflow changed retained bytes");
    }

    if (afterOverflow.LeakCount != beforeOverflow.LeakCount)
    {
        return Fail("capacity overflow changed active leak count");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Setup) != allocations.size())
    {
        return Fail("capacity overflow changed setup budget count");
    }

    for (const MemoryAllocationId allocationId : allocations)
    {
        if (tracker.RecordFree(allocationId, owner, tag) != MemoryAccountingStatus::Success)
        {
            return Fail("fixed-capacity cleanup free failed");
        }
    }

    const auto afterCleanup = tracker.Snapshot();
    if (afterCleanup.HasLeaks())
    {
        return Fail("fixed-capacity cleanup left leaks");
    }

    return 0;
}

int MemoryTrackerEnforcesOwnerAndTagByteCapsWithoutMutation()
{
    CountingMemoryTracker tracker;
    const std::string maxOwner(TRACKED_TEXT_CAP_BYTES, 'O');
    const std::string maxTag(TRACKED_TEXT_CAP_BYTES, 'T');
    const std::string oversizedOwner(TRACKED_TEXT_OVER_CAP_BYTES, 'O');
    const std::string oversizedTag(TRACKED_TEXT_OVER_CAP_BYTES, 'T');

    const auto accepted = tracker.RecordAllocation(
        MemoryOwnerId{maxOwner},
        MemoryTag{maxTag},
        MemoryBudgetClass::Setup,
        SMALL_BYTES,
        ALIGNMENT);
    if (!accepted.Succeeded())
    {
        return Fail("max byte owner/tag allocation failed");
    }

    if (tracker.RecordFree(accepted.AllocationId, MemoryOwnerId{maxOwner}, MemoryTag{maxTag}) !=
        MemoryAccountingStatus::Success)
    {
        return Fail("max byte owner/tag cleanup failed");
    }

    const auto beforeRejects = tracker.Snapshot();
    const auto ownerReject = tracker.RecordAllocation(
        MemoryOwnerId{oversizedOwner},
        MemoryTag{maxTag},
        MemoryBudgetClass::Setup,
        SMALL_BYTES,
        ALIGNMENT);
    if (ownerReject.Status != MemoryAccountingStatus::InvalidOwner)
    {
        return Fail("oversized owner was not rejected");
    }

    const auto tagReject = tracker.RecordAllocation(
        MemoryOwnerId{maxOwner},
        MemoryTag{oversizedTag},
        MemoryBudgetClass::Setup,
        SMALL_BYTES,
        ALIGNMENT);
    if (tagReject.Status != MemoryAccountingStatus::InvalidTag)
    {
        return Fail("oversized tag was not rejected");
    }

    const auto afterRejects = tracker.Snapshot();
    if (afterRejects.AllocationCount != beforeRejects.AllocationCount)
    {
        return Fail("oversized owner/tag reject changed allocation count");
    }

    if (afterRejects.FreeCount != beforeRejects.FreeCount)
    {
        return Fail("oversized owner/tag reject changed free count");
    }

    if (afterRejects.RetainedBytes != beforeRejects.RetainedBytes)
    {
        return Fail("oversized owner/tag reject changed retained bytes");
    }

    if (afterRejects.PeakRetainedBytes != beforeRejects.PeakRetainedBytes)
    {
        return Fail("oversized owner/tag reject changed peak bytes");
    }

    if (afterRejects.LeakCount != beforeRejects.LeakCount)
    {
        return Fail("oversized owner/tag reject changed leak count");
    }

    if (tracker.AllocationCountForBudget(MemoryBudgetClass::Setup) != 1U)
    {
        return Fail("oversized owner/tag reject changed setup budget count");
    }

    return 0;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> testRegistry{
        {TEST_COUNTS, MemoryTrackerCountsAllocationAndFree},
        {TEST_PEAK, MemoryTrackerReportsPeakAndRetainedBytes},
        {TEST_LEAK, MemoryTrackerReportsLeakOnUnreleasedBytes},
        {TEST_UNMATCHED_FREE, MemoryTrackerRejectsUnmatchedFree},
        {TEST_BUDGET_CLASS, MemoryTrackerRecordsBudgetClass},
        {TEST_DISABLED, MemoryDisabledTrackerDoesNotChangeBehavior},
        {TEST_HOT_PATH, MemoryHotPathBudgetFailsOnTrackedAllocation},
        {TEST_FIXED_CAPACITY, MemoryTrackerRejectsBeyondFixedCapacityWithoutMutation},
        {TEST_OWNER_TAG_BYTE_CAPS, MemoryTrackerEnforcesOwnerAndTagByteCapsWithoutMutation}};

    const std::string_view testName(argv[1]);
    const auto testIterator = testRegistry.find(testName);
    if (testIterator == testRegistry.end())
    {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return testIterator->second();
}
