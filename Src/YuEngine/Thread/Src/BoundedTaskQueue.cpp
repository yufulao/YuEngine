// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Src/BoundedTaskQueue.cpp

#include "YuEngine/Thread/BoundedTaskQueue.h"

#include "YuEngine/Memory/MemoryBudgetClass.h"

namespace yuengine::thread {
namespace {
constexpr std::uint64_t INVALID_TASK_ID = 0U;

void ClearTaskQueueCapacityEntry(TaskSchedulerSnapshot &snapshot) {
    snapshot.last_failed_task_id = TaskId{INVALID_TASK_ID};
    snapshot.last_required_queued_task_count = 0U;
    snapshot.last_failed_queue_capacity = 0U;
    snapshot.last_failed_queued_count = 0U;
    snapshot.last_failed_task_completed_count = 0U;
    snapshot.last_failed_task_failed_count = 0U;
    snapshot.last_failed_task_canceled_count = 0U;
}

void RecordTaskQueueCapacityEntry(TaskSchedulerSnapshot &snapshot,
                                  TaskId task_id,
                                  std::size_t queue_capacity,
                                  std::size_t required_queued_task_count) {
    const std::uint64_t completed_count = snapshot.executed_count - snapshot.failed_count;
    snapshot.last_failed_task_id = task_id;
    snapshot.last_required_queued_task_count = required_queued_task_count;
    snapshot.last_failed_queue_capacity = queue_capacity;
    snapshot.last_failed_queued_count = snapshot.pending_count;
    snapshot.last_failed_task_completed_count = completed_count;
    snapshot.last_failed_task_failed_count = snapshot.failed_count;
    snapshot.last_failed_task_canceled_count = snapshot.canceled_count;
}
}

BoundedTaskQueue::BoundedTaskQueue(std::size_t capacity, memory::IMemoryTracker& memory_tracker)
    : records_(capacity),
      memory_tracker_(memory_tracker),
      snapshot_{0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, capacity, capacity, 0U, TaskStatus::Created, false},
      head_index_(0U),
      tail_index_(0U),
      next_task_id_(1U) {
}

TaskResult BoundedTaskQueue::Submit(TaskCallback callback, void* context) {
    if (snapshot_.is_shutdown) {
        ++snapshot_.rejected_count;
        snapshot_.last_status = TaskStatus::Rejected;
        ClearTaskQueueCapacityEntry(snapshot_);
        return RejectResult();
    }

    if (callback == nullptr) {
        ++snapshot_.rejected_count;
        snapshot_.last_status = TaskStatus::Rejected;
        ClearTaskQueueCapacityEntry(snapshot_);
        return RejectResult();
    }

    if (snapshot_.pending_count >= records_.size()) {
        ++snapshot_.rejected_count;
        snapshot_.last_status = TaskStatus::Rejected;
        const std::size_t required_queued_task_count = snapshot_.pending_count + 1U;
        RecordTaskQueueCapacityEntry(snapshot_, TaskId{next_task_id_}, records_.size(), required_queued_task_count);
        return RejectResult();
    }

    const TaskId task_id{next_task_id_};
    ++next_task_id_;

    records_[tail_index_] = TaskRecord{task_id, callback, context, TaskStatus::Queued};
    tail_index_ = (tail_index_ + 1U) % records_.size();
    ++snapshot_.pending_count;
    ++snapshot_.submitted_count;

    if (snapshot_.pending_count > snapshot_.max_queue_depth) {
        snapshot_.max_queue_depth = snapshot_.pending_count;
    }

    ClearTaskQueueCapacityEntry(snapshot_);
    snapshot_.last_status = TaskStatus::Queued;
    return TaskResult{task_id, TaskStatus::Queued};
}

TaskResult BoundedTaskQueue::Drain(InlineTaskExecutor& executor) {
    ++snapshot_.drain_count;
    ClearTaskQueueCapacityEntry(snapshot_);

    TaskResult result = CompleteResult();
    const std::uint64_t allocation_count_before = memory_tracker_.AllocationCountForBudget(memory::MemoryBudgetClass::Job);

    while (snapshot_.pending_count > 0U) {
        TaskRecord& record = records_[head_index_];
        record.status = TaskStatus::Running;

        const TaskStatus execution_status = executor.Execute(record.callback, record.context);
        record.status = execution_status;
        ++snapshot_.executed_count;

        if (execution_status == TaskStatus::Failed) {
            ++snapshot_.failed_count;
            result = TaskResult{record.id, TaskStatus::Failed};
        }

        head_index_ = (head_index_ + 1U) % records_.size();
        --snapshot_.pending_count;
    }

    const std::uint64_t allocation_count_after = memory_tracker_.AllocationCountForBudget(memory::MemoryBudgetClass::Job);
    snapshot_.task_execution_allocation_count += allocation_count_after - allocation_count_before;
    snapshot_.capacity_after_last_drain = records_.capacity();
    snapshot_.last_status = result.status;
    return result;
}

TaskResult BoundedTaskQueue::Shutdown(ShutdownPolicy policy, InlineTaskExecutor& executor) {
    snapshot_.is_shutdown = true;

    if (policy == ShutdownPolicy::CancelQueued) {
        CancelQueuedTasks();
        snapshot_.last_status = TaskStatus::Canceled;
        ClearTaskQueueCapacityEntry(snapshot_);
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
    snapshot_.last_status = TaskStatus::Canceled;
}

TaskResult BoundedTaskQueue::RejectResult() const {
    return TaskResult{TaskId{INVALID_TASK_ID}, TaskStatus::Rejected};
}

TaskResult BoundedTaskQueue::CompleteResult() const {
    return TaskResult{TaskId{INVALID_TASK_ID}, TaskStatus::Completed};
}
}
