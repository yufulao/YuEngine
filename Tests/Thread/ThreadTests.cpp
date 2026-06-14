#include <array>
#include <cstddef>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>

#include "ThreadTestContext.h"
#include "YuEngine/Memory/CountingMemoryTracker.h"
#include "YuEngine/Memory/DisabledMemoryTracker.h"
#include "YuEngine/Thread/BoundedTaskQueue.h"
#include "YuEngine/Thread/InlineTaskExecutor.h"

using BoundedTaskQueue = yuengine::thread::BoundedTaskQueue;
using CountingMemoryTracker = yuengine::memory::CountingMemoryTracker;
using DisabledMemoryTracker = yuengine::memory::DisabledMemoryTracker;
using InlineTaskExecutor = yuengine::thread::InlineTaskExecutor;
using yuengine::thread::ShutdownPolicy;
using yuengine::thread::TaskStatus;
using yuengine::thread::Tests::FixedTraceBuffer;
using yuengine::thread::Tests::ThreadTestContext;

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
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_ENQUEUE_SUCCEEDS, ThreadQueueEnqueueWithinCapacitySucceeds},
        {TEST_ENQUEUE_REJECTS, ThreadQueueEnqueueBeyondCapacityRejects},
        {TEST_FIFO, ThreadDrainExecutesTasksInDeterministicOrder},
        {TEST_FAILURE, ThreadTaskFailureReturnsFailedResult},
        {TEST_SHUTDOWN_REJECTS, ThreadShutdownRejectsNewSubmission},
        {TEST_SHUTDOWN_DRAIN, ThreadShutdownDrainPolicyExecutesQueuedTasks},
        {TEST_SHUTDOWN_CANCEL, ThreadShutdownCancelPolicyCancelsQueuedTasks},
        {TEST_CAPACITY, ThreadQueueCapacityDoesNotGrowDuringFixture},
        {TEST_DIAGNOSTICS_DISABLED, ThreadDiagnosticsDisabledDoesNotChangeBehavior}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
