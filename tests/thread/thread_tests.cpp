#include <array>
#include <cstddef>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>

#include "thread_test_context.h"
#include "yuengine/memory/counting_memory_tracker.h"
#include "yuengine/memory/disabled_memory_tracker.h"
#include "yuengine/thread/bounded_task_queue.h"
#include "yuengine/thread/inline_task_executor.h"

using BoundedTaskQueue = yuengine::thread::BoundedTaskQueue;
using CountingMemoryTracker = yuengine::memory::CountingMemoryTracker;
using DisabledMemoryTracker = yuengine::memory::DisabledMemoryTracker;
using InlineTaskExecutor = yuengine::thread::InlineTaskExecutor;
using ShutdownPolicy = yuengine::thread::ShutdownPolicy;
using TaskStatus = yuengine::thread::TaskStatus;
using FixedTraceBuffer = yuengine::thread::tests::FixedTraceBuffer;
using ThreadTestContext = yuengine::thread::tests::ThreadTestContext;

namespace {
constexpr const char* TEST_ENQUEUE_SUCCEEDS = "Thread_QueueEnqueueWithinCapacity_Succeeds";
constexpr const char* TEST_ENQUEUE_REJECTS = "Thread_QueueEnqueueBeyondCapacity_Rejects";
constexpr const char* TEST_FIFO = "Thread_DrainExecutesTasks_InDeterministicOrder";
constexpr const char* TEST_FAILURE = "Thread_TaskFailure_ReturnsFailedResult";
constexpr const char* TEST_SHUTDOWN_REJECTS = "Thread_ShutdownRejectsNewSubmission";
constexpr const char* TEST_SHUTDOWN_DRAIN = "Thread_ShutdownDrainPolicy_ExecutesQueuedTasks";
constexpr const char* TEST_SHUTDOWN_CANCEL = "Thread_ShutdownCancelPolicy_CancelsQueuedTasks";
constexpr const char* TEST_CAPACITY = "Thread_QueueCapacity_DoesNotGrowDuringFixture";
constexpr const char* TEST_DIAGNOSTICS_DISABLED = "Thread_DiagnosticsDisabled_DoesNotChangeBehavior";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::size_t SMALL_CAPACITY = 2U;
constexpr std::size_t LARGE_CAPACITY = 4U;
constexpr int FIRST_VALUE = 10;
constexpr int SECOND_VALUE = 20;
constexpr int THIRD_VALUE = 30;
using TestFunction = int (*)();

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

template <std::size_t ExpectedCount>
bool TraceEquals(const FixedTraceBuffer& trace, const std::array<int, ExpectedCount>& expected) {
    if (trace.Count != expected.size()) {
        return false;
    }

    for (std::size_t index = 0U; index < trace.Count; ++index) {
        if (trace.Values[index] != expected[index]) {
            return false;
        }
    }

    return true;
}

bool TraceEquals(const FixedTraceBuffer& left, const FixedTraceBuffer& right) {
    if (left.Count != right.Count) {
        return false;
    }

    for (std::size_t index = 0U; index < left.Count; ++index) {
        if (left.Values[index] != right.Values[index]) {
            return false;
        }
    }

    return true;
}

TaskStatus RecordTask(void* context) {
    ThreadTestContext* taskContext = static_cast<ThreadTestContext*>(context);
    if (!taskContext->Trace->Append(taskContext->Value)) {
        return TaskStatus::Failed;
    }

    if (taskContext->ShouldFail) {
        return TaskStatus::Failed;
    }

    return TaskStatus::Completed;
}

int ThreadQueueEnqueueWithinCapacitySucceeds() {
    DisabledMemoryTracker memoryTracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memoryTracker);
    FixedTraceBuffer trace;
    ThreadTestContext context{&trace, FIRST_VALUE, false};

    if (queue.Snapshot().MaxQueueDepth != 0U) {
        return Fail("initial max queue depth was not zero");
    }

    const auto result = queue.Submit(&RecordTask, &context);
    if (result.Status != TaskStatus::Queued) {
        return Fail("submit within capacity did not queue task");
    }

    const auto snapshot = queue.Snapshot();
    if (snapshot.SubmittedCount != 1U) {
        return Fail("submitted count did not increment");
    }

    if (snapshot.PendingCount != 1U) {
        return Fail("pending count did not increment");
    }

    if (snapshot.MaxQueueDepth != 1U) {
        return Fail("max queue depth did not track submitted task");
    }

    return 0;
}

int ThreadQueueEnqueueBeyondCapacityRejects() {
    DisabledMemoryTracker memoryTracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memoryTracker);
    FixedTraceBuffer trace;
    ThreadTestContext firstContext{&trace, FIRST_VALUE, false};
    ThreadTestContext secondContext{&trace, SECOND_VALUE, false};
    ThreadTestContext thirdContext{&trace, THIRD_VALUE, false};

    queue.Submit(&RecordTask, &firstContext);
    queue.Submit(&RecordTask, &secondContext);

    const auto rejectedResult = queue.Submit(&RecordTask, &thirdContext);
    if (rejectedResult.Status != TaskStatus::Rejected) {
        return Fail("submit beyond capacity was not rejected");
    }

    const auto snapshot = queue.Snapshot();
    if (snapshot.RejectedCount != 1U) {
        return Fail("rejected count did not increment");
    }

    if (snapshot.PendingCount != SMALL_CAPACITY) {
        return Fail("overflow changed pending count");
    }

    return 0;
}

int ThreadDrainExecutesTasksInDeterministicOrder() {
    DisabledMemoryTracker memoryTracker;
    BoundedTaskQueue queue(LARGE_CAPACITY, memoryTracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext firstContext{&trace, FIRST_VALUE, false};
    ThreadTestContext secondContext{&trace, SECOND_VALUE, false};
    ThreadTestContext thirdContext{&trace, THIRD_VALUE, false};

    queue.Submit(&RecordTask, &firstContext);
    queue.Submit(&RecordTask, &secondContext);
    queue.Submit(&RecordTask, &thirdContext);

    const auto drainResult = queue.Drain(executor);
    if (drainResult.Status != TaskStatus::Completed) {
        return Fail("drain did not complete");
    }

    const std::array<int, 3U> expectedTrace{FIRST_VALUE, SECOND_VALUE, THIRD_VALUE};
    if (!TraceEquals(trace, expectedTrace)) {
        return Fail("drain order was not FIFO");
    }

    const auto snapshot = queue.Snapshot();
    if (snapshot.ExecutedCount != 3U) {
        return Fail("executed count was wrong");
    }

    if (snapshot.PendingCount != 0U) {
        return Fail("drain left pending tasks");
    }

    return 0;
}

int ThreadTaskFailureReturnsFailedResult() {
    DisabledMemoryTracker memoryTracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memoryTracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext context{&trace, FIRST_VALUE, true};

    queue.Submit(&RecordTask, &context);

    const auto drainResult = queue.Drain(executor);
    if (drainResult.Status != TaskStatus::Failed) {
        return Fail("failed task did not return failed drain result");
    }

    const auto snapshot = queue.Snapshot();
    if (snapshot.FailedCount != 1U) {
        return Fail("failed count did not increment");
    }

    if (snapshot.ExecutedCount != 1U) {
        return Fail("failed task did not execute exactly once");
    }

    return 0;
}

int ThreadShutdownRejectsNewSubmission() {
    DisabledMemoryTracker memoryTracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memoryTracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext context{&trace, FIRST_VALUE, false};

    queue.Shutdown(ShutdownPolicy::DrainQueued, executor);

    const auto submitResult = queue.Submit(&RecordTask, &context);
    if (submitResult.Status != TaskStatus::Rejected) {
        return Fail("submit after shutdown was not rejected");
    }

    const auto snapshot = queue.Snapshot();
    if (!snapshot.IsShutdown) {
        return Fail("shutdown state was not recorded");
    }

    if (snapshot.RejectedCount != 1U) {
        return Fail("shutdown rejection count was wrong");
    }

    return 0;
}

int ThreadShutdownDrainPolicyExecutesQueuedTasks() {
    DisabledMemoryTracker memoryTracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memoryTracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext firstContext{&trace, FIRST_VALUE, false};
    ThreadTestContext secondContext{&trace, SECOND_VALUE, false};

    queue.Submit(&RecordTask, &firstContext);
    queue.Submit(&RecordTask, &secondContext);

    const auto shutdownResult = queue.Shutdown(ShutdownPolicy::DrainQueued, executor);
    if (shutdownResult.Status != TaskStatus::Completed) {
        return Fail("drain shutdown did not complete");
    }

    const std::array<int, 2U> expectedTrace{FIRST_VALUE, SECOND_VALUE};
    if (!TraceEquals(trace, expectedTrace)) {
        return Fail("drain shutdown did not execute queued tasks");
    }

    const auto snapshot = queue.Snapshot();
    if (snapshot.PendingCount != 0U) {
        return Fail("drain shutdown left pending tasks");
    }

    return 0;
}

int ThreadShutdownCancelPolicyCancelsQueuedTasks() {
    DisabledMemoryTracker memoryTracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memoryTracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext firstContext{&trace, FIRST_VALUE, false};
    ThreadTestContext secondContext{&trace, SECOND_VALUE, false};

    queue.Submit(&RecordTask, &firstContext);
    queue.Submit(&RecordTask, &secondContext);

    const auto shutdownResult = queue.Shutdown(ShutdownPolicy::CancelQueued, executor);
    if (shutdownResult.Status != TaskStatus::Canceled) {
        return Fail("cancel shutdown did not return canceled");
    }

    if (!trace.IsEmpty()) {
        return Fail("cancel shutdown executed a queued task");
    }

    const auto snapshot = queue.Snapshot();
    if (snapshot.CanceledCount != 2U) {
        return Fail("cancel count was wrong");
    }

    if (snapshot.PendingCount != 0U) {
        return Fail("cancel shutdown left pending tasks");
    }

    return 0;
}

int ThreadQueueCapacityDoesNotGrowDuringFixture() {
    CountingMemoryTracker memoryTracker;
    BoundedTaskQueue queue(LARGE_CAPACITY, memoryTracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext firstContext{&trace, FIRST_VALUE, false};
    ThreadTestContext secondContext{&trace, SECOND_VALUE, false};

    const std::size_t capacityBefore = queue.Capacity();
    queue.Submit(&RecordTask, &firstContext);
    queue.Submit(&RecordTask, &secondContext);
    queue.Drain(executor);

    const auto snapshot = queue.Snapshot();
    if (capacityBefore != LARGE_CAPACITY) {
        return Fail("queue setup capacity was unexpected");
    }

    if (snapshot.CapacityBeforeFixture != snapshot.CapacityAfterLastDrain) {
        return Fail("queue capacity changed during fixture");
    }

    if (snapshot.TaskExecutionAllocationCount != 0U) {
        return Fail("task execution recorded tracked job allocations");
    }

    return 0;
}

int ThreadDiagnosticsDisabledDoesNotChangeBehavior() {
    DisabledMemoryTracker enabledLikeMemoryTracker;
    BoundedTaskQueue enabledLikeQueue(SMALL_CAPACITY, enabledLikeMemoryTracker);
    InlineTaskExecutor enabledLikeExecutor;
    FixedTraceBuffer enabledLikeTrace;
    ThreadTestContext enabledLikeContext{&enabledLikeTrace, FIRST_VALUE, false};

    enabledLikeQueue.Submit(&RecordTask, &enabledLikeContext);
    enabledLikeQueue.Drain(enabledLikeExecutor);

    DisabledMemoryTracker disabledMemoryTracker;
    BoundedTaskQueue disabledQueue(SMALL_CAPACITY, disabledMemoryTracker);
    InlineTaskExecutor disabledExecutor;
    FixedTraceBuffer disabledTrace;
    ThreadTestContext disabledContext{&disabledTrace, FIRST_VALUE, false};

    disabledQueue.Submit(&RecordTask, &disabledContext);
    disabledQueue.Drain(disabledExecutor);

    if (!TraceEquals(enabledLikeTrace, disabledTrace)) {
        return Fail("diagnostics-disabled fixture changed task behavior");
    }

    const auto enabledLikeSnapshot = enabledLikeQueue.Snapshot();
    const auto disabledSnapshot = disabledQueue.Snapshot();
    if (enabledLikeSnapshot.ExecutedCount != disabledSnapshot.ExecutedCount) {
        return Fail("diagnostics-disabled fixture changed executed count");
    }

    if (enabledLikeSnapshot.FailedCount != disabledSnapshot.FailedCount) {
        return Fail("diagnostics-disabled fixture changed failed count");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> testRegistry{
        {TEST_ENQUEUE_SUCCEEDS, ThreadQueueEnqueueWithinCapacitySucceeds},
        {TEST_ENQUEUE_REJECTS, ThreadQueueEnqueueBeyondCapacityRejects},
        {TEST_FIFO, ThreadDrainExecutesTasksInDeterministicOrder},
        {TEST_FAILURE, ThreadTaskFailureReturnsFailedResult},
        {TEST_SHUTDOWN_REJECTS, ThreadShutdownRejectsNewSubmission},
        {TEST_SHUTDOWN_DRAIN, ThreadShutdownDrainPolicyExecutesQueuedTasks},
        {TEST_SHUTDOWN_CANCEL, ThreadShutdownCancelPolicyCancelsQueuedTasks},
        {TEST_CAPACITY, ThreadQueueCapacityDoesNotGrowDuringFixture},
        {TEST_DIAGNOSTICS_DISABLED, ThreadDiagnosticsDisabledDoesNotChangeBehavior}};

    const std::string_view testName(argv[1]);
    const auto testIterator = testRegistry.find(testName);
    if (testIterator == testRegistry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return testIterator->second();
}
