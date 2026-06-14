#include "YuEngine/Thread/BoundedTaskQueue.h"

#include "YuEngine/Memory/MemoryBudgetClass.h"

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

TaskResult BoundedTaskQueue::Submit(TaskCallback callback, void* context) {
    if (_snapshot.is_shutdown) {
        ++_snapshot.rejected_count;
        return RejectResult();
    }

    if (_snapshot.pending_count >= _records.size()) {
        ++_snapshot.rejected_count;
        return RejectResult();
    }

    const TaskId taskId{_nextTaskId};
    ++_nextTaskId;

    _records[_tailIndex] = TaskRecord{taskId, callback, context, TaskStatus::Queued};
    _tailIndex = (_tailIndex + 1U) % _records.size();
    ++_snapshot.pending_count;
    ++_snapshot.submitted_count;

    if (_snapshot.pending_count > _snapshot.max_queue_depth) {
        _snapshot.max_queue_depth = _snapshot.pending_count;
    }

    return TaskResult{taskId, TaskStatus::Queued};
}

TaskResult BoundedTaskQueue::Drain(InlineTaskExecutor& executor) {
    ++_snapshot.drain_count;

    TaskResult result = CompleteResult();
    const std::uint64_t allocationCountBefore = _memoryTracker.AllocationCountForBudget(memory::MemoryBudgetClass::Job);

    while (_snapshot.pending_count > 0U) {
        TaskRecord& record = _records[_headIndex];
        record.status = TaskStatus::Running;

        const TaskStatus executionStatus = executor.Execute(record.callback, record.context);
        record.status = executionStatus;
        ++_snapshot.executed_count;

        if (executionStatus == TaskStatus::Failed) {
            ++_snapshot.failed_count;
            result = TaskResult{record.id, TaskStatus::Failed};
        }

        _headIndex = (_headIndex + 1U) % _records.size();
        --_snapshot.pending_count;
    }

    const std::uint64_t allocationCountAfter = _memoryTracker.AllocationCountForBudget(memory::MemoryBudgetClass::Job);
    _snapshot.task_execution_allocation_count += allocationCountAfter - allocationCountBefore;
    _snapshot.capacity_after_last_drain = _records.capacity();
    return result;
}

TaskResult BoundedTaskQueue::Shutdown(ShutdownPolicy policy, InlineTaskExecutor& executor) {
    _snapshot.is_shutdown = true;

    if (policy == ShutdownPolicy::CancelQueued) {
        CancelQueuedTasks();
        return TaskResult{TaskId{INVALID_TASK_ID}, TaskStatus::Canceled};
    }

    return Drain(executor);
}

TaskSchedulerSnapshot BoundedTaskQueue::Snapshot() const {
    return _snapshot;
}

std::size_t BoundedTaskQueue::Capacity() const {
    return _records.size();
}

void BoundedTaskQueue::CancelQueuedTasks() {
    while (_snapshot.pending_count > 0U) {
        TaskRecord& record = _records[_headIndex];
        record.status = TaskStatus::Canceled;
        ++_snapshot.canceled_count;
        _headIndex = (_headIndex + 1U) % _records.size();
        --_snapshot.pending_count;
    }

    _snapshot.capacity_after_last_drain = _records.capacity();
}

TaskResult BoundedTaskQueue::RejectResult() const {
    return TaskResult{TaskId{INVALID_TASK_ID}, TaskStatus::Rejected};
}

TaskResult BoundedTaskQueue::CompleteResult() const {
    return TaskResult{TaskId{INVALID_TASK_ID}, TaskStatus::Completed};
}
}
