#include "yuengine/thread/bounded_task_queue.h"

#include "yuengine/memory/memory_budget_class.h"

namespace yuengine::thread {
namespace {
constexpr std::uint64_t INVALID_TASK_ID = 0U;
}

BoundedTaskQueue::BoundedTaskQueue(std::size_t capacity, memory::IMemoryTracker& memoryTracker)
    : _records(capacity),
      _memoryTracker(memoryTracker),
      _snapshot{0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, capacity, capacity, 0U, false},
      _headIndex(0U),
      _tailIndex(0U),
      _nextTaskId(1U) {
}

task_result_t BoundedTaskQueue::Submit(TaskCallback callback, void* context) {
    if (_snapshot.IsShutdown) {
        ++_snapshot.RejectedCount;
        return RejectResult();
    }

    if (_snapshot.PendingCount >= _records.size()) {
        ++_snapshot.RejectedCount;
        return RejectResult();
    }

    const task_id_t taskId{_nextTaskId};
    ++_nextTaskId;

    _records[_tailIndex] = task_record_t{taskId, callback, context, TaskStatus::Queued};
    _tailIndex = (_tailIndex + 1U) % _records.size();
    ++_snapshot.PendingCount;
    ++_snapshot.SubmittedCount;

    if (_snapshot.PendingCount > _snapshot.MaxQueueDepth) {
        _snapshot.MaxQueueDepth = _snapshot.PendingCount;
    }

    return task_result_t{taskId, TaskStatus::Queued};
}

task_result_t BoundedTaskQueue::Drain(InlineTaskExecutor& executor) {
    ++_snapshot.DrainCount;

    task_result_t result = CompleteResult();
    const std::uint64_t allocationCountBefore = _memoryTracker.AllocationCountForBudget(memory::MemoryBudgetClass::Job);

    while (_snapshot.PendingCount > 0U) {
        task_record_t& record = _records[_headIndex];
        record.Status = TaskStatus::Running;

        const TaskStatus executionStatus = executor.Execute(record.Callback, record.Context);
        record.Status = executionStatus;
        ++_snapshot.ExecutedCount;

        if (executionStatus == TaskStatus::Failed) {
            ++_snapshot.FailedCount;
            result = task_result_t{record.Id, TaskStatus::Failed};
        }

        _headIndex = (_headIndex + 1U) % _records.size();
        --_snapshot.PendingCount;
    }

    const std::uint64_t allocationCountAfter = _memoryTracker.AllocationCountForBudget(memory::MemoryBudgetClass::Job);
    _snapshot.TaskExecutionAllocationCount += allocationCountAfter - allocationCountBefore;
    _snapshot.CapacityAfterLastDrain = _records.capacity();
    return result;
}

task_result_t BoundedTaskQueue::Shutdown(ShutdownPolicy policy, InlineTaskExecutor& executor) {
    _snapshot.IsShutdown = true;

    if (policy == ShutdownPolicy::CancelQueued) {
        CancelQueuedTasks();
        return task_result_t{task_id_t{INVALID_TASK_ID}, TaskStatus::Canceled};
    }

    return Drain(executor);
}

task_scheduler_snapshot_t BoundedTaskQueue::Snapshot() const {
    return _snapshot;
}

std::size_t BoundedTaskQueue::Capacity() const {
    return _records.size();
}

void BoundedTaskQueue::CancelQueuedTasks() {
    while (_snapshot.PendingCount > 0U) {
        task_record_t& record = _records[_headIndex];
        record.Status = TaskStatus::Canceled;
        ++_snapshot.CanceledCount;
        _headIndex = (_headIndex + 1U) % _records.size();
        --_snapshot.PendingCount;
    }

    _snapshot.CapacityAfterLastDrain = _records.capacity();
}

task_result_t BoundedTaskQueue::RejectResult() const {
    return task_result_t{task_id_t{INVALID_TASK_ID}, TaskStatus::Rejected};
}

task_result_t BoundedTaskQueue::CompleteResult() const {
    return task_result_t{task_id_t{INVALID_TASK_ID}, TaskStatus::Completed};
}
}
