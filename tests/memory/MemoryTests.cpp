#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

#include "yuengine/memory/CountingMemoryTracker.h"
#include "yuengine/memory/DisabledMemoryTracker.h"

using CountingMemoryTracker = yuengine::memory::CountingMemoryTracker;
using DisabledMemoryTracker = yuengine::memory::DisabledMemoryTracker;
using MemoryAccountingStatus = yuengine::memory::MemoryAccountingStatus;
using MemoryBudgetClass = yuengine::memory::MemoryBudgetClass;
using MemoryOwnerId = yuengine::memory::MemoryOwnerId;
using MemoryTag = yuengine::memory::MemoryTag;
using MemoryAllocationId = yuengine::memory::MemoryAllocationId;

namespace
{
constexpr const char* TEST_COUNTS = "Memory_TrackerCountsAllocationAndFree";
constexpr const char* TEST_PEAK = "Memory_TrackerReportsPeakAndRetainedBytes";
constexpr const char* TEST_LEAK = "Memory_TrackerReportsLeakOnUnreleasedBytes";
constexpr const char* TEST_UNMATCHED_FREE = "Memory_TrackerRejectsUnmatchedFree";
constexpr const char* TEST_BUDGET_CLASS = "Memory_TrackerRecordsBudgetClass";
constexpr const char* TEST_DISABLED = "Memory_DisabledTracker_DoesNotChangeBehavior";
constexpr const char* TEST_HOT_PATH = "Memory_HotPathBudget_FailsOnTrackedAllocation";
constexpr const char* OWNER_PLATFORM = "Platform";
constexpr const char* OWNER_KERNEL = "Kernel";
constexpr const char* TAG_FIXTURE = "Fixture";
constexpr std::size_t SMALL_BYTES = 16U;
constexpr std::size_t MEDIUM_BYTES = 48U;
constexpr std::size_t LARGE_BYTES = 64U;
constexpr std::size_t ALIGNMENT = 8U;

int Fail(const std::string& message)
{
    std::cerr << message << '\n';
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
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return Fail("expected one test name");
    }

    const std::string testName(argv[1]);
    if (testName == TEST_COUNTS)
    {
        return MemoryTrackerCountsAllocationAndFree();
    }

    if (testName == TEST_PEAK)
    {
        return MemoryTrackerReportsPeakAndRetainedBytes();
    }

    if (testName == TEST_LEAK)
    {
        return MemoryTrackerReportsLeakOnUnreleasedBytes();
    }

    if (testName == TEST_UNMATCHED_FREE)
    {
        return MemoryTrackerRejectsUnmatchedFree();
    }

    if (testName == TEST_BUDGET_CLASS)
    {
        return MemoryTrackerRecordsBudgetClass();
    }

    if (testName == TEST_DISABLED)
    {
        return MemoryDisabledTrackerDoesNotChangeBehavior();
    }

    if (testName == TEST_HOT_PATH)
    {
        return MemoryHotPathBudgetFailsOnTrackedAllocation();
    }

    return Fail("unknown test name");
}
