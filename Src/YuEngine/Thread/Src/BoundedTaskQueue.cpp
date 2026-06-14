#include "YuEngine/Thread/BoundedTaskQueue.h"

#include "YuEngine/Memory/MemoryBudgetClass.h"

namespace yuengine::thread {
namespace {
constexpr std::uint64_t INVALID_TASK_ID = 0U;
}

BoundedTaskQueue::BoundedTaskQueue(std::size_t capacity, memory::IMemoryTracker& memoryTracker)
    : records_(capacity),
      memory_tracker_(memoryTracker),
      snapshot_{0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, capacity, capacity, 0U, false},
      head_index_(0U),
      tail_index_(0U),
      next_task_id_(1U) {
}

TaskResult BoundedTaskQueue::Submit(TaskCallback callback, void* context) {
    if (snapshot_.is_shutdown) {
        ++snapshot_.rejected_count;
        return RejectResult();
    }

    if (snapshot_.pending_count >= records_.size()) {
        ++snapshot_.rejected_count;
        return RejectResult();
    }

    const TaskId taskId{next_task_id_};
    ++next_task_id_;

    records_[tail_index_] = TaskRecord{taskId, callback, context, TaskStatus::Queued};
    tail_index_ = (tail_index_ + 1U) % records_.size();
    ++snapshot_.pending_count;
    ++snapshot_.submitted_count;

    if (snapshot_.pending_count > snapshot_.max_queue_depth) {
        snapshot_.max_queue_depth = snapshot_.pending_count;
    }

    return TaskResult{taskId, TaskStatus::Queued};
}

TaskResult BoundedTaskQueue::Drain(InlineTaskExecutor& executor) {
    ++snapshot_.drain_count;

    TaskResult result = CompleteResult();
    const std::uint64_t allocationCountBefore = memory_tracker_.AllocationCountForBudget(memory::MemoryBudgetClass::Job);

    while (snapshot_.pending_count > 0U) {
        TaskRecord& record = records_[head_index_];
        record.status = TaskStatus::Running;

        const TaskStatus executionStatus = executor.Execute(record.callback, record.context);
        record.status = executionStatus;
        ++snapshot_.executed_count;

        if (executionStatus == TaskStatus::Failed) {
            ++snapshot_.failed_count;
            result = TaskResult{record.id, TaskStatus::Failed};
        }

        head_index_ = (head_index_ + 1U) % records_.size();
        --snapshot_.pending_count;
    }

    const std::uint64_t allocationCountAfter = memory_tracker_.AllocationCountForBudget(memory::MemoryBudgetClass::Job);
    snapshot_.task_execution_allocation_count += allocationCountAfter - allocationCountBefore;
    snapshot_.capacity_after_last_drain = records_.capacity();
    return result;
}

TaskResult BoundedTaskQueue::Shutdown(ShutdownPolicy policy, InlineTaskExecutor& executor) {
    snapshot_.is_shutdown = true;

    if (policy == ShutdownPolicy::CancelQueued) {
        CancelQueuedTasks();
        return TaskResult{TaskId{INVALID_TASK_ID}, TaskStatus::Canceled};
    }

    return Drain(executor);
}

TaskSchedulerSnapshot BoundedTaskQueue::Snapshot() const {
    return snapshot_;
}

std::size_t BoundedTaskQueue::Capacity() const {
    return records_.size();
}

void BoundedTaskQueue::CancelQueuedTasks() {
    while (snapshot_.pending_count > 0U) {
        TaskRecord& record = records_[head_index_];
        record.status = TaskStatus::Canceled;
        ++snapshot_.canceled_count;
        head_index_ = (head_index_ + 1U) % records_.size();
        --snapshot_.pending_count;
    }

    snapshot_.capacity_after_last_drain = records_.capacity();
}

TaskResult BoundedTaskQueue::RejectResult() const {
    return TaskResult{TaskId{INVALID_TASK_ID}, TaskStatus::Rejected};
}

TaskResult BoundedTaskQueue::CompleteResult() const {
    return TaskResult{TaskId{INVALID_TASK_ID}, TaskStatus::Completed};
}
}
