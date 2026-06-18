// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Src/ThreadWorker.cpp

#include "YuEngine/Thread/ThreadWorker.h"

#include <condition_variable>
#include <mutex>
#include <new>
#include <thread>
#include <vector>

#include "YuEngine/Thread/TaskRecord.h"

namespace yuengine::thread {
struct ThreadWorkerState final {
    explicit ThreadWorkerState(const ThreadWorkerDesc& input_desc)
        : desc(input_desc),
          work_records(input_desc.work_capacity),
          completion_records(input_desc.completion_capacity) {
        snapshot.work_capacity = input_desc.work_capacity;
        snapshot.completion_capacity = input_desc.completion_capacity;
        snapshot.shutdown_policy = input_desc.default_shutdown_policy;
        snapshot.last_status = ThreadWorkerStatus::Success;
        snapshot.is_initialized = true;
    }

    ThreadWorkerDesc desc;
    std::vector<TaskRecord> work_records;
    std::vector<ThreadWorkerCompletion> completion_records;
    mutable std::mutex mutex;
    std::condition_variable condition;
    std::thread worker;
    ThreadWorkerSnapshot snapshot;
    std::size_t work_head = 0U;
    std::size_t work_tail = 0U;
    std::size_t completion_head = 0U;
    std::size_t completion_tail = 0U;
    std::uint64_t next_task_id = 1U;
};

namespace {
constexpr std::uint64_t INVALID_TASK_ID = 0U;

void SetLastStatusLocked(ThreadWorkerState& state, ThreadWorkerStatus status) {
    state.snapshot.last_status = status;
}

ThreadWorkerStatus RejectSubmitLocked(ThreadWorkerState& state, ThreadWorkerStatus status) {
    ++state.snapshot.rejected_count;
    SetLastStatusLocked(state, status);
    return status;
}

std::size_t ReservedCompletionCountLocked(const ThreadWorkerState& state) {
    std::size_t result = state.snapshot.pending_count;
    result += state.snapshot.running_count;
    result += state.snapshot.completion_pending_count;
    return result;
}

void PushCompletionLocked(ThreadWorkerState& state, TaskId task_id, TaskStatus status) {
    if (state.snapshot.completion_pending_count >= state.completion_records.size()) {
        SetLastStatusLocked(state, ThreadWorkerStatus::CompletionQueueFull);
        return;
    }

    state.completion_records[state.completion_tail] = ThreadWorkerCompletion{task_id, status};
    state.completion_tail = (state.completion_tail + 1U) % state.completion_records.size();
    ++state.snapshot.completion_pending_count;

    if (state.snapshot.completion_pending_count > state.snapshot.max_completion_depth) {
        state.snapshot.max_completion_depth = state.snapshot.completion_pending_count;
    }
}

TaskRecord PopWorkRecordLocked(ThreadWorkerState& state) {
    TaskRecord record = state.work_records[state.work_head];
    state.work_records[state.work_head] = TaskRecord{TaskId{INVALID_TASK_ID}, nullptr, nullptr, TaskStatus::Created};
    state.work_head = (state.work_head + 1U) % state.work_records.size();
    --state.snapshot.pending_count;
    ++state.snapshot.running_count;
    ++state.snapshot.started_count;
    return record;
}

void CompleteWorkRecord(ThreadWorkerState& state, TaskId task_id, TaskStatus status) {
    std::lock_guard<std::mutex> lock(state.mutex);

    if (state.snapshot.running_count > 0U) {
        --state.snapshot.running_count;
    }

    if (status == TaskStatus::Completed) {
        ++state.snapshot.completed_count;
    }

    if (status == TaskStatus::Failed) {
        ++state.snapshot.failed_count;
    }

    if (status == TaskStatus::Canceled) {
        ++state.snapshot.canceled_count;
    }

    PushCompletionLocked(state, task_id, status);
    SetLastStatusLocked(state, ThreadWorkerStatus::Success);
    state.condition.notify_all();
}

void CancelQueuedTasksLocked(ThreadWorkerState& state) {
    while (state.snapshot.pending_count > 0U) {
        TaskRecord record = state.work_records[state.work_head];
        state.work_records[state.work_head] = TaskRecord{TaskId{INVALID_TASK_ID}, nullptr, nullptr, TaskStatus::Created};
        state.work_head = (state.work_head + 1U) % state.work_records.size();
        --state.snapshot.pending_count;
        record.status = TaskStatus::Canceled;
        ++state.snapshot.canceled_count;
        PushCompletionLocked(state, record.id, TaskStatus::Canceled);
    }
}

bool ShouldWakeWorker(const ThreadWorkerState& state) {
    if (state.snapshot.pending_count > 0U) {
        return true;
    }

    return state.snapshot.is_shutdown;
}

void RunWorker(ThreadWorkerState* state) {
    if (state == nullptr) {
        return;
    }

    bool should_continue = true;
    while (should_continue) {
        TaskRecord record{TaskId{INVALID_TASK_ID}, nullptr, nullptr, TaskStatus::Created};
        bool has_record = false;

        {
            std::unique_lock<std::mutex> lock(state->mutex);
            state->condition.wait(lock, [state]() {
                return ShouldWakeWorker(*state);
            });

            if (state->snapshot.pending_count > 0U) {
                record = PopWorkRecordLocked(*state);
                has_record = true;
            }

            if (!has_record && state->snapshot.is_shutdown) {
                should_continue = false;
            }
        }

        if (!has_record) {
            continue;
        }

        TaskStatus execution_status = TaskStatus::Failed;
        if (record.callback != nullptr) {
            execution_status = record.callback(record.context);
        }

        CompleteWorkRecord(*state, record.id, execution_status);
    }
}
}

ThreadWorker::ThreadWorker()
    : state_(nullptr) {
}

ThreadWorker::~ThreadWorker() {
    if (state_ == nullptr) {
        return;
    }

    Shutdown(state_->desc.default_shutdown_policy);
    delete state_;
    state_ = nullptr;
}

ThreadWorkerStatus ThreadWorker::Initialize(const ThreadWorkerDesc& desc) {
    if (state_ != nullptr) {
        return ThreadWorkerStatus::AlreadyInitialized;
    }

    if (desc.work_capacity == 0U) {
        return ThreadWorkerStatus::InvalidDescriptor;
    }

    if (desc.completion_capacity == 0U) {
        return ThreadWorkerStatus::InvalidDescriptor;
    }

    state_ = new (std::nothrow) ThreadWorkerState(desc);
    if (state_ == nullptr) {
        return ThreadWorkerStatus::AllocationFailure;
    }

    return ThreadWorkerStatus::Success;
}

ThreadWorkerStatus ThreadWorker::Start() {
    if (state_ == nullptr) {
        return ThreadWorkerStatus::NotInitialized;
    }

    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        if (state_->snapshot.is_started) {
            return ThreadWorkerStatus::AlreadyStarted;
        }

        if (state_->snapshot.is_shutdown) {
            return ThreadWorkerStatus::StopRequested;
        }
    }

    state_->worker = std::thread(RunWorker, state_);

    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->snapshot.is_started = true;
        state_->snapshot.is_joined = false;
        SetLastStatusLocked(*state_, ThreadWorkerStatus::Success);
    }

    state_->condition.notify_all();
    return ThreadWorkerStatus::Success;
}

ThreadWorkerStatus ThreadWorker::Submit(TaskCallback callback, void* context, TaskId* task_id) {
    if (task_id != nullptr) {
        *task_id = TaskId{INVALID_TASK_ID};
    }

    if (state_ == nullptr) {
        return ThreadWorkerStatus::NotInitialized;
    }

    if (callback == nullptr) {
        return ThreadWorkerStatus::InvalidArgument;
    }

    std::lock_guard<std::mutex> lock(state_->mutex);
    if (state_->snapshot.is_shutdown) {
        return RejectSubmitLocked(*state_, ThreadWorkerStatus::StopRequested);
    }

    if (!state_->snapshot.is_started) {
        return RejectSubmitLocked(*state_, ThreadWorkerStatus::NotStarted);
    }

    if (state_->snapshot.pending_count >= state_->work_records.size()) {
        return RejectSubmitLocked(*state_, ThreadWorkerStatus::QueueFull);
    }

    const std::size_t reserved_completion_count = ReservedCompletionCountLocked(*state_);
    if (reserved_completion_count >= state_->completion_records.size()) {
        return RejectSubmitLocked(*state_, ThreadWorkerStatus::CompletionQueueFull);
    }

    const TaskId new_task_id{state_->next_task_id};
    ++state_->next_task_id;

    state_->work_records[state_->work_tail] = TaskRecord{new_task_id, callback, context, TaskStatus::Queued};
    state_->work_tail = (state_->work_tail + 1U) % state_->work_records.size();
    ++state_->snapshot.pending_count;
    ++state_->snapshot.submitted_count;

    if (state_->snapshot.pending_count > state_->snapshot.max_queue_depth) {
        state_->snapshot.max_queue_depth = state_->snapshot.pending_count;
    }

    if (task_id != nullptr) {
        *task_id = new_task_id;
    }

    SetLastStatusLocked(*state_, ThreadWorkerStatus::Success);
    state_->condition.notify_one();
    return ThreadWorkerStatus::Success;
}

ThreadWorkerStatus ThreadWorker::RequestStop(ShutdownPolicy policy) {
    if (state_ == nullptr) {
        return ThreadWorkerStatus::NotInitialized;
    }

    std::lock_guard<std::mutex> lock(state_->mutex);
    if (!state_->snapshot.is_started) {
        return ThreadWorkerStatus::NotStarted;
    }

    if (state_->snapshot.is_shutdown) {
        return ThreadWorkerStatus::StopRequested;
    }

    state_->snapshot.is_shutdown = true;
    state_->snapshot.shutdown_policy = policy;

    if (policy == ShutdownPolicy::CancelQueued) {
        CancelQueuedTasksLocked(*state_);
    }

    SetLastStatusLocked(*state_, ThreadWorkerStatus::Success);
    state_->condition.notify_all();
    return ThreadWorkerStatus::Success;
}

ThreadWorkerStatus ThreadWorker::Join() {
    if (state_ == nullptr) {
        return ThreadWorkerStatus::NotInitialized;
    }

    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        if (!state_->snapshot.is_started && state_->snapshot.is_joined) {
            return ThreadWorkerStatus::ShutdownComplete;
        }

        if (!state_->snapshot.is_started) {
            return ThreadWorkerStatus::NotStarted;
        }

        if (!state_->snapshot.is_shutdown) {
            return ThreadWorkerStatus::StopRequired;
        }
    }

    state_->condition.notify_all();
    if (state_->worker.joinable()) {
        state_->worker.join();
    }

    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->snapshot.is_started = false;
        state_->snapshot.is_joined = true;
        state_->snapshot.work_capacity_after_shutdown = state_->work_records.capacity();
        state_->snapshot.completion_capacity_after_shutdown = state_->completion_records.capacity();
        SetLastStatusLocked(*state_, ThreadWorkerStatus::ShutdownComplete);
    }

    return ThreadWorkerStatus::ShutdownComplete;
}

ThreadWorkerStatus ThreadWorker::Shutdown(ShutdownPolicy policy) {
    ThreadWorkerStatus stop_status = RequestStop(policy);
    if (stop_status == ThreadWorkerStatus::NotInitialized) {
        return stop_status;
    }

    if (stop_status == ThreadWorkerStatus::NotStarted) {
        return stop_status;
    }

    ThreadWorkerStatus join_status = Join();
    return join_status;
}

ThreadWorkerStatus ThreadWorker::DrainCompletions(
    ThreadWorkerCompletion* output_records,
    std::size_t output_capacity,
    std::size_t* written_count) {
    if (written_count == nullptr) {
        return ThreadWorkerStatus::InvalidArgument;
    }

    *written_count = 0U;

    if (state_ == nullptr) {
        return ThreadWorkerStatus::NotInitialized;
    }

    if (output_capacity > 0U && output_records == nullptr) {
        return ThreadWorkerStatus::InvalidArgument;
    }

    std::lock_guard<std::mutex> lock(state_->mutex);
    if (state_->snapshot.completion_pending_count > 0U && output_capacity == 0U) {
        return ThreadWorkerStatus::CompletionQueueFull;
    }

    while (state_->snapshot.completion_pending_count > 0U && *written_count < output_capacity) {
        output_records[*written_count] = state_->completion_records[state_->completion_head];
        state_->completion_records[state_->completion_head] = ThreadWorkerCompletion{};
        state_->completion_head = (state_->completion_head + 1U) % state_->completion_records.size();
        --state_->snapshot.completion_pending_count;
        ++state_->snapshot.drained_completion_count;
        ++(*written_count);
    }

    if (state_->snapshot.completion_pending_count > 0U) {
        SetLastStatusLocked(*state_, ThreadWorkerStatus::CompletionQueueFull);
        return ThreadWorkerStatus::CompletionQueueFull;
    }

    SetLastStatusLocked(*state_, ThreadWorkerStatus::Success);
    return ThreadWorkerStatus::Success;
}

ThreadWorkerSnapshot ThreadWorker::Snapshot() const {
    if (state_ == nullptr) {
        return ThreadWorkerSnapshot{};
    }

    std::lock_guard<std::mutex> lock(state_->mutex);
    return state_->snapshot;
}
}
