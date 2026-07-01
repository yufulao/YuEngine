// 模块：Tests Thread
// 文件：Tests/Thread/ThreadTests.cpp

#include <array>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include "ThreadTestContext.h"
#include "YuEngine/Memory/CountingMemoryTracker.h"
#include "YuEngine/Memory/DisabledMemoryTracker.h"
#include "YuEngine/Thread/BoundedTaskQueue.h"
#include "YuEngine/Thread/InlineTaskExecutor.h"
#include "YuEngine/Thread/ThreadWorker.h"

using BoundedTaskQueue = yuengine::thread::BoundedTaskQueue;
using CountingMemoryTracker = yuengine::memory::CountingMemoryTracker;
using DisabledMemoryTracker = yuengine::memory::DisabledMemoryTracker;
using InlineTaskExecutor = yuengine::thread::InlineTaskExecutor;
using TaskSchedulerSnapshot = yuengine::thread::TaskSchedulerSnapshot;
using ThreadWorker = yuengine::thread::ThreadWorker;
using ThreadWorkerCompletion = yuengine::thread::ThreadWorkerCompletion;
using ThreadWorkerDesc = yuengine::thread::ThreadWorkerDesc;
using ThreadWorkerSnapshot = yuengine::thread::ThreadWorkerSnapshot;
using yuengine::thread::TaskId;
using yuengine::thread::ThreadWorkerStatus;
using yuengine::thread::ShutdownPolicy;
using yuengine::thread::TaskStatus;
using yuengine::thread::Tests::FixedTraceBuffer;
using yuengine::thread::Tests::ThreadTestContext;

namespace {
constexpr const char* TEST_ENQUEUE_SUCCEEDS = "Thread_QueueEnqueueWithinCapacity_Succeeds";
constexpr const char* TEST_ENQUEUE_REJECTS = "Thread_QueueEnqueueBeyondCapacity_Rejects";
constexpr const char* TEST_QUEUE_CAPACITY_ENTRY = "Thread_QueueCapacityOverflow_ReportsRejectedEntry";
constexpr const char* TEST_FIFO = "Thread_DrainExecutesTasks_InDeterministicOrder";
constexpr const char* TEST_FAILURE = "Thread_TaskFailure_ReturnsFailedResult";
constexpr const char* TEST_SHUTDOWN_REJECTS = "Thread_ShutdownRejectsNewSubmission";
constexpr const char* TEST_SHUTDOWN_DRAIN = "Thread_ShutdownDrainPolicy_ExecutesQueuedTasks";
constexpr const char* TEST_SHUTDOWN_CANCEL = "Thread_ShutdownCancelPolicy_CancelsQueuedTasks";
constexpr const char* TEST_CAPACITY = "Thread_QueueCapacity_DoesNotGrowDuringFixture";
constexpr const char* TEST_QUEUE_LAST_STATUS = "Thread_QueueLastStatus_TracksSubmitDrainCancel";
constexpr const char* TEST_DIAGNOSTICS_DISABLED = "Thread_DiagnosticsDisabled_DoesNotChangeBehavior";
constexpr const char* TEST_WORKER_BEFORE_START = "Thread_WorkerSubmitBeforeStart_ReturnsExplicitStatus";
constexpr const char* TEST_WORKER_DRAIN = "Thread_WorkerDrainShutdown_WritesCompletionRecords";
constexpr const char* TEST_WORKER_CANCEL = "Thread_WorkerCancelShutdown_CancelsQueuedWork";
constexpr const char* TEST_WORKER_COMPLETION_CAPACITY = "Thread_WorkerCompletionCapacity_RejectsWithoutMutation";
constexpr const char* TEST_WORKER_COMPLETION_CAPACITY_ENTRY =
    "Thread_WorkerCompletionCapacityEntry_ClearsOnNonQueueCapacity";
constexpr const char* TEST_WORKER_COMPLETION_DRAIN = "Thread_WorkerCompletionDrain_UsesCallerStorageLimit";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::size_t SMALL_CAPACITY = 2U;
constexpr std::size_t LARGE_CAPACITY = 4U;
constexpr int FIRST_VALUE = 10;
constexpr int SECOND_VALUE = 20;
constexpr int THIRD_VALUE = 30;
using TestFunction = int (*)();

struct BlockingThreadContext {
    FixedTraceBuffer *trace = nullptr;
    int value = 0;
    bool entered = false;
    bool release = false;
    std::mutex mutex;
    std::condition_variable condition;
};

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

template <std::size_t ExpectedCount>
bool TraceEquals(const FixedTraceBuffer &trace, const std::array<int, ExpectedCount> &expected) {
    if (trace.count != expected.size()) {
        return false;
    }

    for (std::size_t index = 0U; index < trace.count; ++index) {
        if (trace.values[index] != expected[index]) {
            return false;
        }
    }

    return true;
}

bool TraceEquals(const FixedTraceBuffer &left, const FixedTraceBuffer &right) {
    if (left.count != right.count) {
        return false;
    }

    for (std::size_t index = 0U; index < left.count; ++index) {
        if (left.values[index] != right.values[index]) {
            return false;
        }
    }

    return true;
}

int ExpectCompletionQueueCapacityEntryClear(const ThreadWorkerSnapshot &snapshot) {
    if (snapshot.last_failed_completion_task_id.value != 0U) {
        return Fail("completion queue capacity entry retained failed task id");
    }

    if (snapshot.last_failed_completion_status != TaskStatus::Created) {
        return Fail("completion queue capacity entry retained failed task status");
    }

    if (snapshot.last_failed_completion_queue_capacity != 0U) {
        return Fail("completion queue capacity entry retained failed queue capacity");
    }

    if (snapshot.last_failed_completion_pending_count != 0U) {
        return Fail("completion queue capacity entry retained failed pending count");
    }

    return 0;
}

int ExpectCompletionQueueCapacityEntryMatches(
    const ThreadWorkerSnapshot &snapshot,
    std::uint64_t task_id,
    TaskStatus status,
    std::size_t queue_capacity,
    std::size_t completion_pending_count,
    std::size_t required_completion_count) {
    if (snapshot.last_failed_completion_task_id.value != task_id) {
        return Fail("completion queue capacity entry missed task id");
    }

    if (snapshot.last_failed_completion_status != status) {
        return Fail("completion queue capacity entry missed task status");
    }

    if (snapshot.last_failed_completion_queue_capacity != queue_capacity) {
        return Fail("completion queue capacity entry missed queue capacity");
    }

    if (snapshot.last_failed_completion_pending_count != completion_pending_count) {
        return Fail("completion queue capacity entry missed pending count");
    }

    if (snapshot.last_required_completion_count != required_completion_count) {
        return Fail("completion queue capacity entry missed required count");
    }

    return 0;
}

TaskStatus RecordTask(void* context) {
    ThreadTestContext *task_context = static_cast<ThreadTestContext *>(context);
    if (!task_context->trace->Append(task_context->value)) {
        return TaskStatus::Failed;
    }

    if (task_context->should_fail) {
        return TaskStatus::Failed;
    }

    return TaskStatus::Completed;
}

TaskStatus BlockingRecordTask(void* context) {
    BlockingThreadContext *task_context = static_cast<BlockingThreadContext *>(context);
    if (task_context == nullptr) {
        return TaskStatus::Failed;
    }

    {
        std::unique_lock<std::mutex> lock(task_context->mutex);
        task_context->entered = true;
        task_context->condition.notify_all();
        task_context->condition.wait(lock, [task_context]() {
            return task_context->release;
        });
    }

    if (task_context->trace == nullptr) {
        return TaskStatus::Failed;
    }

    if (!task_context->trace->Append(task_context->value)) {
        return TaskStatus::Failed;
    }

    return TaskStatus::Completed;
}

bool TaskQueueCapacityEntryMatches(const TaskSchedulerSnapshot &snapshot,
                                   std::uint64_t task_id,
                                   std::size_t queued_count,
                                   std::uint64_t completed_count,
                                   std::uint64_t failed_count,
                                   std::uint64_t canceled_count) {
    if (snapshot.last_failed_task_id.value != task_id) {
        return false;
    }

    if (snapshot.last_required_queued_task_count != queued_count + 1U) {
        return false;
    }

    if (snapshot.last_failed_queue_capacity != SMALL_CAPACITY) {
        return false;
    }

    if (snapshot.last_failed_queued_count != queued_count) {
        return false;
    }

    if (snapshot.last_failed_task_completed_count != completed_count) {
        return false;
    }

    if (snapshot.last_failed_task_failed_count != failed_count) {
        return false;
    }

    if (snapshot.last_failed_task_canceled_count != canceled_count) {
        return false;
    }

    return true;
}

bool TaskQueueCapacityEntryIsClear(const TaskSchedulerSnapshot &snapshot) {
    if (snapshot.last_failed_task_id.value != 0U) {
        return false;
    }

    if (snapshot.last_required_queued_task_count != 0U) {
        return false;
    }

    if (snapshot.last_failed_queue_capacity != 0U) {
        return false;
    }

    if (snapshot.last_failed_queued_count != 0U) {
        return false;
    }

    if (snapshot.last_failed_task_completed_count != 0U) {
        return false;
    }

    if (snapshot.last_failed_task_failed_count != 0U) {
        return false;
    }

    if (snapshot.last_failed_task_canceled_count != 0U) {
        return false;
    }

    return true;
}

void WaitForBlockingTask(BlockingThreadContext &context) {
    std::unique_lock<std::mutex> lock(context.mutex);
    context.condition.wait(lock, [&context]() {
        return context.entered;
    });
}

void ReleaseBlockingTask(BlockingThreadContext &context) {
    {
        std::lock_guard<std::mutex> lock(context.mutex);
        context.release = true;
    }

    context.condition.notify_all();
}

int ThreadQueueEnqueueWithinCapacitySucceeds() {
    DisabledMemoryTracker memory_tracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memory_tracker);
    FixedTraceBuffer trace;
    ThreadTestContext context{&trace, FIRST_VALUE, false};

    if (queue.Snapshot().max_queue_depth != 0U) {
        return Fail("initial max queue depth was not zero");
    }

    const auto result = queue.Submit(&RecordTask, &context);
    if (result.status != TaskStatus::Queued) {
        return Fail("submit within capacity did not queue task");
    }

    const auto snapshot = queue.Snapshot();
    if (snapshot.submitted_count != 1U) {
        return Fail("submitted count did not increment");
    }

    if (snapshot.pending_count != 1U) {
        return Fail("pending count did not increment");
    }

    if (snapshot.max_queue_depth != 1U) {
        return Fail("max queue depth did not track submitted task");
    }

    return 0;
}

int ThreadQueueEnqueueBeyondCapacityRejects() {
    DisabledMemoryTracker memory_tracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memory_tracker);
    FixedTraceBuffer trace;
    ThreadTestContext first_context{&trace, FIRST_VALUE, false};
    ThreadTestContext second_context{&trace, SECOND_VALUE, false};
    ThreadTestContext third_context{&trace, THIRD_VALUE, false};

    queue.Submit(&RecordTask, &first_context);
    queue.Submit(&RecordTask, &second_context);

    const auto rejected_result = queue.Submit(&RecordTask, &third_context);
    if (rejected_result.status != TaskStatus::Rejected) {
        return Fail("submit beyond capacity was not rejected");
    }

    const auto snapshot = queue.Snapshot();
    if (snapshot.rejected_count != 1U) {
        return Fail("rejected count did not increment");
    }

    if (snapshot.pending_count != SMALL_CAPACITY) {
        return Fail("overflow changed pending count");
    }

    return 0;
}

int ThreadQueueCapacityOverflowReportsRejectedEntry() {
    DisabledMemoryTracker memory_tracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memory_tracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext first_context{&trace, FIRST_VALUE, false};
    ThreadTestContext second_context{&trace, SECOND_VALUE, false};
    ThreadTestContext third_context{&trace, THIRD_VALUE, false};
    ThreadTestContext fourth_context{&trace, FIRST_VALUE, false};
    ThreadTestContext fifth_context{&trace, SECOND_VALUE, false};

    if (queue.Submit(&RecordTask, &first_context).status != TaskStatus::Queued) {
        return Fail("first capacity entry setup submit failed");
    }

    if (queue.Submit(&RecordTask, &second_context).status != TaskStatus::Queued) {
        return Fail("second capacity entry setup submit failed");
    }

    const TaskSchedulerSnapshot before_snapshot = queue.Snapshot();
    const auto first_rejected_result = queue.Submit(&RecordTask, &third_context);
    if (first_rejected_result.status != TaskStatus::Rejected) {
        return Fail("capacity entry submit was not rejected");
    }

    const TaskSchedulerSnapshot first_rejected_snapshot = queue.Snapshot();
    if (!TaskQueueCapacityEntryMatches(first_rejected_snapshot, 3U, before_snapshot.pending_count, 0U, 0U, 0U)) {
        return Fail("capacity entry did not record initial rejection identity");
    }

    if (first_rejected_snapshot.pending_count != before_snapshot.pending_count) {
        return Fail("capacity entry rejection changed pending count");
    }

    if (first_rejected_snapshot.submitted_count != before_snapshot.submitted_count) {
        return Fail("capacity entry rejection changed submitted count");
    }

    if (first_rejected_snapshot.rejected_count != before_snapshot.rejected_count + 1U) {
        return Fail("capacity entry rejection did not increment reject count");
    }

    const auto invalid_callback_result = queue.Submit(nullptr, &third_context);
    if (invalid_callback_result.status != TaskStatus::Rejected) {
        return Fail("invalid callback submit was not rejected");
    }

    if (!TaskQueueCapacityEntryIsClear(queue.Snapshot())) {
        return Fail("invalid callback submit kept stale capacity entry");
    }

    const auto second_rejected_result = queue.Submit(&RecordTask, &third_context);
    if (second_rejected_result.status != TaskStatus::Rejected) {
        return Fail("second capacity entry submit was not rejected");
    }

    if (queue.Drain(executor).status != TaskStatus::Completed) {
        return Fail("drain did not clear capacity entry fixture");
    }

    if (!TaskQueueCapacityEntryIsClear(queue.Snapshot())) {
        return Fail("drain kept stale capacity entry");
    }

    const std::array<int, 2U> expected_trace{FIRST_VALUE, SECOND_VALUE};
    if (!TraceEquals(trace, expected_trace)) {
        return Fail("capacity entry drain executed unexpected tasks");
    }

    if (queue.Submit(&RecordTask, &third_context).status != TaskStatus::Queued) {
        return Fail("post-drain submit did not queue task");
    }

    if (!TaskQueueCapacityEntryIsClear(queue.Snapshot())) {
        return Fail("successful submit kept stale capacity entry");
    }

    if (queue.Submit(&RecordTask, &fourth_context).status != TaskStatus::Queued) {
        return Fail("post-drain second submit did not queue task");
    }

    const TaskSchedulerSnapshot completed_before_snapshot = queue.Snapshot();
    const auto completed_rejected_result = queue.Submit(&RecordTask, &fifth_context);
    if (completed_rejected_result.status != TaskStatus::Rejected) {
        return Fail("completed count capacity submit was not rejected");
    }

    const TaskSchedulerSnapshot completed_rejected_snapshot = queue.Snapshot();
    if (!TaskQueueCapacityEntryMatches(completed_rejected_snapshot, 5U, completed_before_snapshot.pending_count, 2U, 0U, 0U)) {
        return Fail("capacity entry did not record completed-count rejection identity");
    }

    if (queue.Shutdown(ShutdownPolicy::CancelQueued, executor).status != TaskStatus::Canceled) {
        return Fail("cancel shutdown did not clear capacity entry fixture");
    }

    if (!TaskQueueCapacityEntryIsClear(queue.Snapshot())) {
        return Fail("cancel shutdown kept stale capacity entry");
    }

    return 0;
}

int ThreadDrainExecutesTasksInDeterministicOrder() {
    DisabledMemoryTracker memory_tracker;
    BoundedTaskQueue queue(LARGE_CAPACITY, memory_tracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext first_context{&trace, FIRST_VALUE, false};
    ThreadTestContext second_context{&trace, SECOND_VALUE, false};
    ThreadTestContext third_context{&trace, THIRD_VALUE, false};

    queue.Submit(&RecordTask, &first_context);
    queue.Submit(&RecordTask, &second_context);
    queue.Submit(&RecordTask, &third_context);

    const auto drain_result = queue.Drain(executor);
    if (drain_result.status != TaskStatus::Completed) {
        return Fail("drain did not complete");
    }

    const std::array<int, 3U> expected_trace{FIRST_VALUE, SECOND_VALUE, THIRD_VALUE};
    if (!TraceEquals(trace, expected_trace)) {
        return Fail("drain order was not FIFO");
    }

    const auto snapshot = queue.Snapshot();
    if (snapshot.executed_count != 3U) {
        return Fail("executed count was wrong");
    }

    if (snapshot.pending_count != 0U) {
        return Fail("drain left pending tasks");
    }

    return 0;
}

int ThreadTaskFailureReturnsFailedResult() {
    DisabledMemoryTracker memory_tracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memory_tracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext context{&trace, FIRST_VALUE, true};

    queue.Submit(&RecordTask, &context);

    const auto drain_result = queue.Drain(executor);
    if (drain_result.status != TaskStatus::Failed) {
        return Fail("failed task did not return failed drain result");
    }

    const auto snapshot = queue.Snapshot();
    if (snapshot.failed_count != 1U) {
        return Fail("failed count did not increment");
    }

    if (snapshot.executed_count != 1U) {
        return Fail("failed task did not execute exactly once");
    }

    return 0;
}

int ThreadShutdownRejectsNewSubmission() {
    DisabledMemoryTracker memory_tracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memory_tracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext context{&trace, FIRST_VALUE, false};

    queue.Shutdown(ShutdownPolicy::DrainQueued, executor);

    const auto submit_result = queue.Submit(&RecordTask, &context);
    if (submit_result.status != TaskStatus::Rejected) {
        return Fail("submit after shutdown was not rejected");
    }

    const auto snapshot = queue.Snapshot();
    if (!snapshot.is_shutdown) {
        return Fail("shutdown state was not recorded");
    }

    if (snapshot.rejected_count != 1U) {
        return Fail("shutdown rejection count was wrong");
    }

    return 0;
}

int ThreadShutdownDrainPolicyExecutesQueuedTasks() {
    DisabledMemoryTracker memory_tracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memory_tracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext first_context{&trace, FIRST_VALUE, false};
    ThreadTestContext second_context{&trace, SECOND_VALUE, false};

    queue.Submit(&RecordTask, &first_context);
    queue.Submit(&RecordTask, &second_context);

    const auto shutdown_result = queue.Shutdown(ShutdownPolicy::DrainQueued, executor);
    if (shutdown_result.status != TaskStatus::Completed) {
        return Fail("drain shutdown did not complete");
    }

    const std::array<int, 2U> expected_trace{FIRST_VALUE, SECOND_VALUE};
    if (!TraceEquals(trace, expected_trace)) {
        return Fail("drain shutdown did not execute queued tasks");
    }

    const auto snapshot = queue.Snapshot();
    if (snapshot.pending_count != 0U) {
        return Fail("drain shutdown left pending tasks");
    }

    return 0;
}

int ThreadShutdownCancelPolicyCancelsQueuedTasks() {
    DisabledMemoryTracker memory_tracker;
    BoundedTaskQueue queue(SMALL_CAPACITY, memory_tracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext first_context{&trace, FIRST_VALUE, false};
    ThreadTestContext second_context{&trace, SECOND_VALUE, false};

    queue.Submit(&RecordTask, &first_context);
    queue.Submit(&RecordTask, &second_context);

    const auto shutdown_result = queue.Shutdown(ShutdownPolicy::CancelQueued, executor);
    if (shutdown_result.status != TaskStatus::Canceled) {
        return Fail("cancel shutdown did not return canceled");
    }

    if (!trace.IsEmpty()) {
        return Fail("cancel shutdown executed a queued task");
    }

    const auto snapshot = queue.Snapshot();
    if (snapshot.canceled_count != 2U) {
        return Fail("cancel count was wrong");
    }

    if (snapshot.pending_count != 0U) {
        return Fail("cancel shutdown left pending tasks");
    }

    return 0;
}

int ThreadQueueCapacityDoesNotGrowDuringFixture() {
    CountingMemoryTracker memory_tracker;
    BoundedTaskQueue queue(LARGE_CAPACITY, memory_tracker);
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;
    ThreadTestContext first_context{&trace, FIRST_VALUE, false};
    ThreadTestContext second_context{&trace, SECOND_VALUE, false};

    const std::size_t capacity_before = queue.Capacity();
    queue.Submit(&RecordTask, &first_context);
    queue.Submit(&RecordTask, &second_context);
    queue.Drain(executor);

    const auto snapshot = queue.Snapshot();
    if (capacity_before != LARGE_CAPACITY) {
        return Fail("queue setup capacity was unexpected");
    }

    if (snapshot.capacity_before_fixture != snapshot.capacity_after_last_drain) {
        return Fail("queue capacity changed during fixture");
    }

    if (snapshot.task_execution_allocation_count != 0U) {
        return Fail("task execution recorded tracked job allocations");
    }

    return 0;
}

int ThreadQueueLastStatusTracksSubmitDrainCancel() {
    DisabledMemoryTracker memory_tracker;
    InlineTaskExecutor executor;
    FixedTraceBuffer trace;

    BoundedTaskQueue queue(SMALL_CAPACITY, memory_tracker);
    if (queue.Snapshot().last_status != TaskStatus::Created) {
        return Fail("initial queue last status was not created");
    }

    ThreadTestContext first_context{&trace, FIRST_VALUE, false};
    if (queue.Submit(&RecordTask, &first_context).status != TaskStatus::Queued) {
        return Fail("last status fixture submit failed");
    }

    if (queue.Snapshot().last_status != TaskStatus::Queued) {
        return Fail("queued task did not update last status");
    }

    ThreadTestContext second_context{&trace, SECOND_VALUE, false};
    ThreadTestContext third_context{&trace, THIRD_VALUE, false};
    queue.Submit(&RecordTask, &second_context);
    if (queue.Submit(&RecordTask, &third_context).status != TaskStatus::Rejected) {
        return Fail("last status fixture rejection failed");
    }

    auto snapshot = queue.Snapshot();
    if (snapshot.last_status != TaskStatus::Rejected) {
        return Fail("rejected task did not update last status");
    }

    if (snapshot.rejected_count != 1U) {
        return Fail("last status fixture changed rejected count");
    }

    if (queue.Drain(executor).status != TaskStatus::Completed) {
        return Fail("last status fixture drain failed");
    }

    snapshot = queue.Snapshot();
    if (snapshot.last_status != TaskStatus::Completed) {
        return Fail("completed drain did not update last status");
    }

    if (snapshot.executed_count != 2U) {
        return Fail("last status fixture changed executed count");
    }

    BoundedTaskQueue failed_queue(SMALL_CAPACITY, memory_tracker);
    ThreadTestContext failed_context{&trace, FIRST_VALUE, true};
    failed_queue.Submit(&RecordTask, &failed_context);
    if (failed_queue.Drain(executor).status != TaskStatus::Failed) {
        return Fail("last status fixture failed task did not fail drain");
    }

    snapshot = failed_queue.Snapshot();
    if (snapshot.last_status != TaskStatus::Failed) {
        return Fail("failed drain did not update last status");
    }

    if (snapshot.failed_count != 1U) {
        return Fail("last status fixture changed failed count");
    }

    BoundedTaskQueue canceled_queue(SMALL_CAPACITY, memory_tracker);
    ThreadTestContext cancel_context{&trace, FIRST_VALUE, false};
    canceled_queue.Submit(&RecordTask, &cancel_context);
    if (canceled_queue.Shutdown(ShutdownPolicy::CancelQueued, executor).status != TaskStatus::Canceled) {
        return Fail("last status fixture cancel shutdown failed");
    }

    snapshot = canceled_queue.Snapshot();
    if (snapshot.last_status != TaskStatus::Canceled) {
        return Fail("canceled shutdown did not update last status");
    }

    if (snapshot.canceled_count != 1U) {
        return Fail("last status fixture changed canceled count");
    }

    return 0;
}

int ThreadDiagnosticsDisabledDoesNotChangeBehavior() {
    DisabledMemoryTracker enabled_like_memory_tracker;
    BoundedTaskQueue enabled_like_queue(SMALL_CAPACITY, enabled_like_memory_tracker);
    InlineTaskExecutor enabled_like_executor;
    FixedTraceBuffer enabled_like_trace;
    ThreadTestContext enabled_like_context{&enabled_like_trace, FIRST_VALUE, false};

    enabled_like_queue.Submit(&RecordTask, &enabled_like_context);
    enabled_like_queue.Drain(enabled_like_executor);

    DisabledMemoryTracker disabled_memory_tracker;
    BoundedTaskQueue disabled_queue(SMALL_CAPACITY, disabled_memory_tracker);
    InlineTaskExecutor disabled_executor;
    FixedTraceBuffer disabled_trace;
    ThreadTestContext disabled_context{&disabled_trace, FIRST_VALUE, false};

    disabled_queue.Submit(&RecordTask, &disabled_context);
    disabled_queue.Drain(disabled_executor);

    if (!TraceEquals(enabled_like_trace, disabled_trace)) {
        return Fail("diagnostics-disabled fixture changed task behavior");
    }

    const auto enabled_like_snapshot = enabled_like_queue.Snapshot();
    const auto disabled_snapshot = disabled_queue.Snapshot();
    if (enabled_like_snapshot.executed_count != disabled_snapshot.executed_count) {
        return Fail("diagnostics-disabled fixture changed executed count");
    }

    if (enabled_like_snapshot.failed_count != disabled_snapshot.failed_count) {
        return Fail("diagnostics-disabled fixture changed failed count");
    }

    return 0;
}

int ThreadWorkerSubmitBeforeStartReturnsExplicitStatus() {
    ThreadWorker worker;
    ThreadWorkerDesc desc;
    desc.work_capacity = SMALL_CAPACITY;
    desc.completion_capacity = SMALL_CAPACITY;

    const ThreadWorkerStatus init_status = worker.Initialize(desc);
    if (init_status != ThreadWorkerStatus::Success) {
        return Fail("worker initialize failed");
    }

    FixedTraceBuffer trace;
    ThreadTestContext context{&trace, FIRST_VALUE, false};
    const ThreadWorkerStatus submit_status = worker.Submit(&RecordTask, &context);
    if (submit_status != ThreadWorkerStatus::NotStarted) {
        return Fail("submit before start did not return not-started");
    }

    const auto snapshot = worker.Snapshot();
    if (snapshot.rejected_count != 1U) {
        return Fail("submit before start did not count rejection");
    }

    return 0;
}

int ThreadWorkerDrainShutdownWritesCompletionRecords() {
    ThreadWorker worker;
    ThreadWorkerDesc desc;
    desc.work_capacity = LARGE_CAPACITY;
    desc.completion_capacity = LARGE_CAPACITY;

    const ThreadWorkerStatus init_status = worker.Initialize(desc);
    if (init_status != ThreadWorkerStatus::Success) {
        return Fail("worker initialize failed");
    }

    const ThreadWorkerStatus start_status = worker.Start();
    if (start_status != ThreadWorkerStatus::Success) {
        return Fail("worker start failed");
    }

    FixedTraceBuffer trace;
    ThreadTestContext first_context{&trace, FIRST_VALUE, false};
    ThreadTestContext second_context{&trace, SECOND_VALUE, false};
    worker.Submit(&RecordTask, &first_context);
    worker.Submit(&RecordTask, &second_context);

    const ThreadWorkerStatus shutdown_status = worker.Shutdown(ShutdownPolicy::DrainQueued);
    if (shutdown_status != ThreadWorkerStatus::ShutdownComplete) {
        return Fail("drain shutdown did not complete");
    }

    std::array<ThreadWorkerCompletion, 2U> completions{};
    std::size_t written_count = 0U;
    const ThreadWorkerStatus drain_status = worker.DrainCompletions(
        completions.data(),
        completions.size(),
        &written_count);
    if (drain_status != ThreadWorkerStatus::Success) {
        return Fail("completion drain failed");
    }

    if (written_count != completions.size()) {
        return Fail("completion count was wrong");
    }

    std::size_t completed_count = 0U;
    for (const ThreadWorkerCompletion &completion : completions) {
        if (completion.status == TaskStatus::Completed) {
            ++completed_count;
        }
    }

    if (completed_count != completions.size()) {
        return Fail("completion records did not return completed tasks");
    }

    const std::array<int, 2U> expected_trace{FIRST_VALUE, SECOND_VALUE};
    if (!TraceEquals(trace, expected_trace)) {
        return Fail("worker drain did not execute queued tasks");
    }

    const auto snapshot = worker.Snapshot();
    if (snapshot.completed_count != completions.size()) {
        return Fail("worker snapshot completed count was wrong");
    }

    if (snapshot.pending_count != 0U) {
        return Fail("worker drain left pending tasks");
    }

    if (snapshot.last_status != ThreadWorkerStatus::Success) {
        return Fail("clean completion drain changed last status");
    }

    return 0;
}

int ThreadWorkerCancelShutdownCancelsQueuedWork() {
    ThreadWorker worker;
    ThreadWorkerDesc desc;
    desc.work_capacity = LARGE_CAPACITY;
    desc.completion_capacity = LARGE_CAPACITY;

    const ThreadWorkerStatus init_status = worker.Initialize(desc);
    if (init_status != ThreadWorkerStatus::Success) {
        return Fail("worker initialize failed");
    }

    const ThreadWorkerStatus start_status = worker.Start();
    if (start_status != ThreadWorkerStatus::Success) {
        return Fail("worker start failed");
    }

    FixedTraceBuffer trace;
    BlockingThreadContext blocking_context;
    blocking_context.trace = &trace;
    blocking_context.value = FIRST_VALUE;
    ThreadTestContext queued_context{&trace, SECOND_VALUE, false};

    worker.Submit(&BlockingRecordTask, &blocking_context);
    worker.Submit(&RecordTask, &queued_context);
    WaitForBlockingTask(blocking_context);

    const ThreadWorkerStatus stop_status = worker.RequestStop(ShutdownPolicy::CancelQueued);
    if (stop_status != ThreadWorkerStatus::Success) {
        return Fail("cancel stop request failed");
    }

    ReleaseBlockingTask(blocking_context);

    const ThreadWorkerStatus join_status = worker.Join();
    if (join_status != ThreadWorkerStatus::ShutdownComplete) {
        return Fail("cancel shutdown join failed");
    }

    std::array<ThreadWorkerCompletion, 2U> completions{};
    std::size_t written_count = 0U;
    worker.DrainCompletions(completions.data(), completions.size(), &written_count);
    if (written_count != completions.size()) {
        return Fail("cancel completion count was wrong");
    }

    std::size_t canceled_count = 0U;
    std::size_t completed_count = 0U;
    for (const ThreadWorkerCompletion &completion : completions) {
        if (completion.status == TaskStatus::Canceled) {
            ++canceled_count;
        }

        if (completion.status == TaskStatus::Completed) {
            ++completed_count;
        }
    }

    if (canceled_count != 1U) {
        return Fail("cancel shutdown did not cancel queued work");
    }

    if (completed_count != 1U) {
        return Fail("cancel shutdown did not complete running work");
    }

    const std::array<int, 1U> expected_trace{FIRST_VALUE};
    if (!TraceEquals(trace, expected_trace)) {
        return Fail("cancel shutdown executed canceled work");
    }

    return 0;
}

int ThreadWorkerCompletionCapacityRejectsWithoutMutation() {
    ThreadWorker worker;
    ThreadWorkerDesc desc;
    desc.work_capacity = LARGE_CAPACITY;
    desc.completion_capacity = 1U;

    const ThreadWorkerStatus init_status = worker.Initialize(desc);
    if (init_status != ThreadWorkerStatus::Success) {
        return Fail("worker initialize failed");
    }

    const ThreadWorkerStatus start_status = worker.Start();
    if (start_status != ThreadWorkerStatus::Success) {
        return Fail("worker start failed");
    }

    FixedTraceBuffer trace;
    BlockingThreadContext blocking_context;
    blocking_context.trace = &trace;
    blocking_context.value = FIRST_VALUE;
    ThreadTestContext rejected_context{&trace, SECOND_VALUE, false};

    const ThreadWorkerStatus first_submit = worker.Submit(&BlockingRecordTask, &blocking_context);
    if (first_submit != ThreadWorkerStatus::Success) {
        return Fail("first submit failed");
    }

    WaitForBlockingTask(blocking_context);

    const ThreadWorkerStatus second_submit = worker.Submit(&RecordTask, &rejected_context);
    if (second_submit != ThreadWorkerStatus::CompletionQueueFull) {
        return Fail("completion capacity did not reject second submit");
    }

    const auto rejected_snapshot = worker.Snapshot();
    if (rejected_snapshot.last_required_completion_count != 2U) {
        return Fail("completion capacity did not report required completion count");
    }

    if (rejected_snapshot.completed_count != 0U) {
        return Fail("completion capacity rejection changed completed count");
    }

    ReleaseBlockingTask(blocking_context);
    worker.Shutdown(ShutdownPolicy::DrainQueued);

    std::array<ThreadWorkerCompletion, 1U> completions{};
    std::size_t written_count = 0U;
    worker.DrainCompletions(completions.data(), completions.size(), &written_count);
    if (written_count != 1U) {
        return Fail("completion capacity test wrote wrong count");
    }

    const auto snapshot = worker.Snapshot();
    if (snapshot.rejected_count != 1U) {
        return Fail("completion capacity rejection was not counted");
    }

    if (snapshot.completed_count != 1U) {
        return Fail("completion capacity changed accepted work count");
    }

    return 0;
}

int ThreadWorkerCompletionCapacityEntryClearsOnNonQueueCapacity() {
    ThreadWorker worker;
    ThreadWorkerDesc desc;
    desc.work_capacity = LARGE_CAPACITY;
    desc.completion_capacity = 1U;

    const ThreadWorkerStatus init_status = worker.Initialize(desc);
    if (init_status != ThreadWorkerStatus::Success) {
        return Fail("capacity entry worker initialize failed");
    }

    const ThreadWorkerStatus start_status = worker.Start();
    if (start_status != ThreadWorkerStatus::Success) {
        return Fail("capacity entry worker start failed");
    }

    FixedTraceBuffer trace;
    BlockingThreadContext blocking_context;
    blocking_context.trace = &trace;
    blocking_context.value = FIRST_VALUE;
    ThreadTestContext rejected_context{&trace, SECOND_VALUE, false};

    TaskId accepted_task_id{0U};
    const ThreadWorkerStatus first_submit = worker.Submit(
        &BlockingRecordTask,
        &blocking_context,
        &accepted_task_id);
    if (first_submit != ThreadWorkerStatus::Success) {
        return Fail("capacity entry first submit failed");
    }

    WaitForBlockingTask(blocking_context);

    TaskId rejected_task_id{0U};
    const ThreadWorkerStatus second_submit = worker.Submit(
        &RecordTask,
        &rejected_context,
        &rejected_task_id);
    if (second_submit != ThreadWorkerStatus::CompletionQueueFull) {
        return Fail("capacity entry second submit returned wrong status");
    }

    if (accepted_task_id.value != 1U) {
        return Fail("capacity entry accepted task id was unexpected");
    }

    if (rejected_task_id.value != 0U) {
        return Fail("capacity entry wrote rejected task output id");
    }

    ThreadWorkerSnapshot snapshot = worker.Snapshot();
    int check_result = ExpectCompletionQueueCapacityEntryMatches(
        snapshot,
        2U,
        TaskStatus::Rejected,
        1U,
        0U,
        2U);
    if (check_result != 0) {
        return check_result;
    }

    if (snapshot.completion_pending_count != 0U) {
        return Fail("capacity entry rejection mutated completion storage");
    }

    const ThreadWorkerStatus invalid_status = worker.Submit(nullptr, nullptr);
    if (invalid_status != ThreadWorkerStatus::InvalidArgument) {
        return Fail("capacity entry invalid submit returned wrong status");
    }

    snapshot = worker.Snapshot();
    check_result = ExpectCompletionQueueCapacityEntryClear(snapshot);
    if (check_result != 0) {
        return check_result;
    }

    const ThreadWorkerStatus refreshed_submit = worker.Submit(&RecordTask, &rejected_context);
    if (refreshed_submit != ThreadWorkerStatus::CompletionQueueFull) {
        return Fail("capacity entry refresh submit returned wrong status");
    }

    ReleaseBlockingTask(blocking_context);
    const ThreadWorkerStatus shutdown_status = worker.Shutdown(ShutdownPolicy::DrainQueued);
    if (shutdown_status != ThreadWorkerStatus::ShutdownComplete) {
        return Fail("capacity entry shutdown returned wrong status");
    }

    snapshot = worker.Snapshot();
    check_result = ExpectCompletionQueueCapacityEntryClear(snapshot);
    if (check_result != 0) {
        return check_result;
    }

    std::array<ThreadWorkerCompletion, 1U> small_output{};
    std::size_t small_written_count = 0U;
    const ThreadWorkerStatus drain_small_status = worker.DrainCompletions(
        small_output.data(),
        0U,
        &small_written_count);
    if (drain_small_status != ThreadWorkerStatus::CompletionQueueFull) {
        return Fail("capacity entry small drain returned wrong status");
    }

    snapshot = worker.Snapshot();
    check_result = ExpectCompletionQueueCapacityEntryClear(snapshot);
    if (check_result != 0) {
        return check_result;
    }

    std::array<ThreadWorkerCompletion, 1U> completion_output{};
    std::size_t written_count = 0U;
    const ThreadWorkerStatus drain_status = worker.DrainCompletions(
        completion_output.data(),
        completion_output.size(),
        &written_count);
    if (drain_status != ThreadWorkerStatus::Success) {
        return Fail("capacity entry final drain returned wrong status");
    }

    if (written_count != 1U) {
        return Fail("capacity entry final drain wrote wrong count");
    }

    snapshot = worker.Snapshot();
    check_result = ExpectCompletionQueueCapacityEntryClear(snapshot);
    if (check_result != 0) {
        return check_result;
    }

    const ThreadWorkerStatus stopped_submit = worker.Submit(&RecordTask, &rejected_context);
    if (stopped_submit != ThreadWorkerStatus::StopRequested) {
        return Fail("capacity entry stopped submit returned wrong status");
    }

    snapshot = worker.Snapshot();
    check_result = ExpectCompletionQueueCapacityEntryClear(snapshot);
    if (check_result != 0) {
        return check_result;
    }

    ThreadWorker not_started_worker;
    ThreadWorkerDesc not_started_desc;
    not_started_desc.work_capacity = SMALL_CAPACITY;
    not_started_desc.completion_capacity = SMALL_CAPACITY;
    if (not_started_worker.Initialize(not_started_desc) != ThreadWorkerStatus::Success) {
        return Fail("capacity entry not-started worker initialize failed");
    }

    ThreadTestContext not_started_context{&trace, THIRD_VALUE, false};
    const ThreadWorkerStatus not_started_status = not_started_worker.Submit(
        &RecordTask,
        &not_started_context);
    if (not_started_status != ThreadWorkerStatus::NotStarted) {
        return Fail("capacity entry not-started submit returned wrong status");
    }

    snapshot = not_started_worker.Snapshot();
    check_result = ExpectCompletionQueueCapacityEntryClear(snapshot);
    if (check_result != 0) {
        return check_result;
    }

    return 0;
}

int ThreadWorkerCompletionDrainUsesCallerStorageLimit() {
    ThreadWorker worker;
    ThreadWorkerDesc desc;
    desc.work_capacity = LARGE_CAPACITY;
    desc.completion_capacity = LARGE_CAPACITY;

    const ThreadWorkerStatus init_status = worker.Initialize(desc);
    if (init_status != ThreadWorkerStatus::Success) {
        return Fail("worker initialize failed");
    }

    const ThreadWorkerStatus start_status = worker.Start();
    if (start_status != ThreadWorkerStatus::Success) {
        return Fail("worker start failed");
    }

    FixedTraceBuffer trace;
    ThreadTestContext first_context{&trace, FIRST_VALUE, false};
    ThreadTestContext second_context{&trace, SECOND_VALUE, false};
    worker.Submit(&RecordTask, &first_context);
    worker.Submit(&RecordTask, &second_context);
    worker.Shutdown(ShutdownPolicy::DrainQueued);

    std::array<ThreadWorkerCompletion, 1U> first_drain{};
    std::size_t first_written_count = 0U;
    const ThreadWorkerStatus first_drain_status = worker.DrainCompletions(
        first_drain.data(),
        first_drain.size(),
        &first_written_count);
    if (first_drain_status != ThreadWorkerStatus::CompletionQueueFull) {
        return Fail("small completion output did not keep remaining records");
    }

    if (first_written_count != 0U) {
        return Fail("small completion output wrote records before success");
    }

    const auto rejected_snapshot = worker.Snapshot();
    if (rejected_snapshot.completion_pending_count != 2U) {
        return Fail("small completion output mutated pending records");
    }

    if (rejected_snapshot.drained_completion_count != 0U) {
        return Fail("small completion output changed drained count");
    }

    if (rejected_snapshot.last_required_completion_count != 2U) {
        return Fail("small completion output did not report required completion count");
    }

    std::array<ThreadWorkerCompletion, 2U> second_drain{};
    std::size_t second_written_count = 0U;
    const ThreadWorkerStatus second_drain_status = worker.DrainCompletions(
        second_drain.data(),
        second_drain.size(),
        &second_written_count);
    if (second_drain_status != ThreadWorkerStatus::Success) {
        return Fail("second completion drain failed");
    }

    if (second_written_count != second_drain.size()) {
        return Fail("second completion drain wrote wrong count");
    }

    const auto snapshot = worker.Snapshot();
    if (snapshot.drained_completion_count != 2U) {
        return Fail("drained completion count was wrong");
    }

    if (snapshot.last_required_completion_count != 2U) {
        return Fail("second completion drain lost required completion count");
    }

    if (snapshot.last_status != ThreadWorkerStatus::CompletionQueueFull) {
        return Fail("second completion drain erased prior failure status");
    }

    if (snapshot.work_capacity != snapshot.work_capacity_after_shutdown) {
        return Fail("work capacity changed during worker fixture");
    }

    if (snapshot.completion_capacity != snapshot.completion_capacity_after_shutdown) {
        return Fail("completion capacity changed during worker fixture");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_ENQUEUE_SUCCEEDS, ThreadQueueEnqueueWithinCapacitySucceeds},
        {TEST_ENQUEUE_REJECTS, ThreadQueueEnqueueBeyondCapacityRejects},
        {TEST_QUEUE_CAPACITY_ENTRY, ThreadQueueCapacityOverflowReportsRejectedEntry},
        {TEST_FIFO, ThreadDrainExecutesTasksInDeterministicOrder},
        {TEST_FAILURE, ThreadTaskFailureReturnsFailedResult},
        {TEST_SHUTDOWN_REJECTS, ThreadShutdownRejectsNewSubmission},
        {TEST_SHUTDOWN_DRAIN, ThreadShutdownDrainPolicyExecutesQueuedTasks},
        {TEST_SHUTDOWN_CANCEL, ThreadShutdownCancelPolicyCancelsQueuedTasks},
        {TEST_CAPACITY, ThreadQueueCapacityDoesNotGrowDuringFixture},
        {TEST_QUEUE_LAST_STATUS, ThreadQueueLastStatusTracksSubmitDrainCancel},
        {TEST_DIAGNOSTICS_DISABLED, ThreadDiagnosticsDisabledDoesNotChangeBehavior},
        {TEST_WORKER_BEFORE_START, ThreadWorkerSubmitBeforeStartReturnsExplicitStatus},
        {TEST_WORKER_DRAIN, ThreadWorkerDrainShutdownWritesCompletionRecords},
        {TEST_WORKER_CANCEL, ThreadWorkerCancelShutdownCancelsQueuedWork},
        {TEST_WORKER_COMPLETION_CAPACITY, ThreadWorkerCompletionCapacityRejectsWithoutMutation},
        {TEST_WORKER_COMPLETION_CAPACITY_ENTRY, ThreadWorkerCompletionCapacityEntryClearsOnNonQueueCapacity},
        {TEST_WORKER_COMPLETION_DRAIN, ThreadWorkerCompletionDrainUsesCallerStorageLimit}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
