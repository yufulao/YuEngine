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
          completion_records(input_desc.completion_capacity),
          completion_snapshots(input_desc.completion_capacity) {
        snapshot.work_capacity = input_desc.work_capacity;
        snapshot.completion_capacity = input_desc.completion_capacity;
        snapshot.shutdown_policy = input_desc.default_shutdown_policy;
        snapshot.last_status = ThreadWorkerStatus::Success;
        snapshot.is_initialized = true;
    }

    ThreadWorkerDesc desc;
    std::vector<TaskRecord> work_records;
    std::vector<ThreadWorkerCompletion> completion_records;
    std::vector<ThreadWorkerCompletion> completion_snapshots;
    mutable std::mutex mutex;
    std::condition_variable condition;
    std::thread worker;
    ThreadWorkerSnapshot snapshot;
    std::size_t work_head = 0U;
    std::size_t work_tail = 0U;
    std::size_t completion_head = 0U;
    std::size_t completion_tail = 0U;
    std::size_t completion_snapshot_tail = 0U;
    std::size_t completion_snapshot_count = 0U;
    std::uint64_t next_task_id = 1U;
};

namespace {
constexpr std::uint64_t INVALID_TASK_ID = 0U;

void SetLastStatusLocked(ThreadWorkerState& state, ThreadWorkerStatus status) {
    state.snapshot.last_status = status;
}

void ClearCompletionQueueCapacityEntryLocked(ThreadWorkerState &state) {
    state.snapshot.last_failed_completion_task_id = TaskId{INVALID_TASK_ID};
    state.snapshot.last_failed_completion_status = TaskStatus::Created;
    state.snapshot.last_failed_completion_queue_capacity = 0U;
    state.snapshot.last_failed_completion_pending_count = 0U;
}

void ClearCompletionDrainOutputEntryLocked(ThreadWorkerState &state) {
    state.snapshot.last_failed_drain_completion_index = 0U;
    state.snapshot.last_failed_drain_completion_id = TaskId{INVALID_TASK_ID};
    state.snapshot.last_failed_drain_completion_status = TaskStatus::Created;
}

void ClearCompletionFailureEntriesLocked(ThreadWorkerState &state) {
    ClearCompletionQueueCapacityEntryLocked(state);
    ClearCompletionDrainOutputEntryLocked(state);
}

void SetCompletionQueueFullLocked(ThreadWorkerState &state, std::size_t required_completion_count) {
    state.snapshot.last_required_completion_count = required_completion_count;
    ClearCompletionQueueCapacityEntryLocked(state);
    SetLastStatusLocked(state, ThreadWorkerStatus::CompletionQueueFull);
}

void RecordCompletionQueueCapacityEntryLocked(
    ThreadWorkerState &state,
    TaskId task_id,
    TaskStatus status,
    std::size_t required_completion_count) {
    state.snapshot.last_required_completion_count = required_completion_count;
    state.snapshot.last_failed_completion_task_id = task_id;
    state.snapshot.last_failed_completion_status = status;
    state.snapshot.last_failed_completion_queue_capacity = state.completion_records.size();
    state.snapshot.last_failed_completion_pending_count = state.snapshot.completion_pending_count;
    SetLastStatusLocked(state, ThreadWorkerStatus::CompletionQueueFull);
}

void SetCompletionDrainOutputEntryLocked(ThreadWorkerState& state, std::size_t output_capacity) {
    const std::size_t failed_record_index = (state.completion_head + output_capacity) % state.completion_records.size();
    const ThreadWorkerCompletion &failed_completion = state.completion_records[failed_record_index];
    state.snapshot.last_failed_drain_completion_index = output_capacity;
    state.snapshot.last_failed_drain_completion_id = failed_completion.task_id;
    state.snapshot.last_failed_drain_completion_status = failed_completion.status;
}

ThreadWorkerStatus RejectSubmitLocked(ThreadWorkerState& state, ThreadWorkerStatus status) {
    ++state.snapshot.rejected_count;
    if (status != ThreadWorkerStatus::CompletionQueueFull) {
        ClearCompletionQueueCapacityEntryLocked(state);
    }

    ClearCompletionDrainOutputEntryLocked(state);
    SetLastStatusLocked(state, status);
    return status;
}

bool ShouldPreserveStatusAfterDrain(ThreadWorkerStatus status) {
    if (status == ThreadWorkerStatus::Success) {
        return false;
    }

    if (status == ThreadWorkerStatus::ShutdownComplete) {
        return false;
    }

    return true;
}

std::size_t ReservedCompletionCountLocked(const ThreadWorkerState& state) {
    std::size_t result = state.snapshot.pending_count;
    result += state.snapshot.running_count;
    result += state.snapshot.completion_pending_count;
    return result;
}

void SaveCompletionSnapshotLocked(ThreadWorkerState &state, ThreadWorkerCompletion completion) {
    if (state.completion_snapshots.empty()) {
        return;
    }

    state.completion_snapshots[state.completion_snapshot_tail] = completion;
    state.completion_snapshot_tail = (state.completion_snapshot_tail + 1U) % state.completion_snapshots.size();
    if (state.completion_snapshot_count < state.completion_snapshots.size()) {
        ++state.completion_snapshot_count;
    }
}

bool PushCompletionLocked(ThreadWorkerState &state, TaskId task_id, TaskStatus status) {
    if (state.snapshot.completion_pending_count >= state.completion_records.size()) {
        const std::size_t required_completion_count = state.snapshot.completion_pending_count + 1U;
        RecordCompletionQueueCapacityEntryLocked(state, task_id, status, required_completion_count);
        ClearCompletionDrainOutputEntryLocked(state);
        return false;
    }

    const ThreadWorkerCompletion completion{task_id, status};
    state.completion_records[state.completion_tail] = completion;
    state.completion_tail = (state.completion_tail + 1U) % state.completion_records.size();
    ++state.snapshot.completion_pending_count;
    SaveCompletionSnapshotLocked(state, completion);

    if (state.snapshot.completion_pending_count > state.snapshot.max_completion_depth) {
        state.snapshot.max_completion_depth = state.snapshot.completion_pending_count;
    }

    ClearCompletionFailureEntriesLocked(state);
    return true;
}

bool FindPendingWorkCompletionLocked(
    const ThreadWorkerState& state,
    TaskId task_id,
    ThreadWorkerCompletion* output_record) {
    std::size_t record_index = state.work_head;
    for (std::size_t count = 0U; count < state.snapshot.pending_count; ++count) {
        const TaskRecord& record = state.work_records[record_index];
        if (record.id.value == task_id.value) {
            *output_record = ThreadWorkerCompletion{record.id, record.status};
            return true;
        }

        record_index = (record_index + 1U) % state.work_records.size();
    }

    return false;
}

bool FindPendingCompletionLocked(
    const ThreadWorkerState& state,
    TaskId task_id,
    ThreadWorkerCompletion* output_record) {
    std::size_t record_index = state.completion_head;
    for (std::size_t count = 0U; count < state.snapshot.completion_pending_count; ++count) {
        const ThreadWorkerCompletion& completion = state.completion_records[record_index];
        if (completion.task_id.value == task_id.value) {
            *output_record = completion;
            return true;
        }

        record_index = (record_index + 1U) % state.completion_records.size();
    }

    return false;
}

bool FindCompletionSnapshotLocked(
    const ThreadWorkerState& state,
    TaskId task_id,
    ThreadWorkerCompletion* output_record) {
    for (std::size_t index = 0U; index < state.completion_snapshot_count; ++index) {
        const ThreadWorkerCompletion& completion = state.completion_snapshots[index];
        if (completion.task_id.value == task_id.value) {
            *output_record = completion;
            return true;
        }
    }

    return false;
}

bool IsCompletionEnumerationStatus(TaskStatus status) {
    if (status == TaskStatus::Completed) {
        return true;
    }

    if (status == TaskStatus::Failed) {
        return true;
    }

    if (status == TaskStatus::Canceled) {
        return true;
    }

    return false;
}

std::size_t FirstCompletionSnapshotIndexLocked(const ThreadWorkerState& state) {
    if (state.completion_snapshot_count < state.completion_snapshots.size()) {
        return 0U;
    }

    return state.completion_snapshot_tail;
}

std::size_t CountCompletionSnapshotsByStatusLocked(const ThreadWorkerState& state, TaskStatus status) {
    if (state.completion_snapshot_count == 0U) {
        return 0U;
    }

    const std::size_t snapshot_capacity = state.completion_snapshots.size();
    std::size_t result = 0U;
    std::size_t record_index = FirstCompletionSnapshotIndexLocked(state);
    for (std::size_t count = 0U; count < state.completion_snapshot_count; ++count) {
        const ThreadWorkerCompletion& completion = state.completion_snapshots[record_index];
        if (completion.status == status) {
            ++result;
        }

        record_index = (record_index + 1U) % snapshot_capacity;
    }

    return result;
}

void WriteCompletionSnapshotsByStatusLocked(
    const ThreadWorkerState& state,
    TaskStatus status,
    ThreadWorkerCompletion* output_records) {
    if (state.completion_snapshot_count == 0U) {
        return;
    }

    const std::size_t snapshot_capacity = state.completion_snapshots.size();
    std::size_t output_index = 0U;
    std::size_t record_index = FirstCompletionSnapshotIndexLocked(state);
    for (std::size_t count = 0U; count < state.completion_snapshot_count; ++count) {
        const ThreadWorkerCompletion& completion = state.completion_snapshots[record_index];
        if (completion.status == status) {
            output_records[output_index] = completion;
            ++output_index;
        }

        record_index = (record_index + 1U) % snapshot_capacity;
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

    const bool completion_pushed = PushCompletionLocked(state, task_id, status);
    if (completion_pushed) {
        SetLastStatusLocked(state, ThreadWorkerStatus::Success);
    }

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
            ClearCompletionFailureEntriesLocked(*state_);
            return ThreadWorkerStatus::AlreadyStarted;
        }

        if (state_->snapshot.is_shutdown) {
            ClearCompletionFailureEntriesLocked(*state_);
            return ThreadWorkerStatus::StopRequested;
        }
    }

    state_->worker = std::thread(RunWorker, state_);

    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->snapshot.is_started = true;
        state_->snapshot.is_joined = false;
        ClearCompletionFailureEntriesLocked(*state_);
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
        std::lock_guard<std::mutex> lock(state_->mutex);
        ClearCompletionFailureEntriesLocked(*state_);
        SetLastStatusLocked(*state_, ThreadWorkerStatus::InvalidArgument);
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
    const std::size_t required_completion_count = reserved_completion_count + 1U;
    if (required_completion_count > state_->completion_records.size()) {
        const TaskId rejected_task_id{state_->next_task_id};
        RecordCompletionQueueCapacityEntryLocked(
            *state_,
            rejected_task_id,
            TaskStatus::Rejected,
            required_completion_count);
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

    ClearCompletionFailureEntriesLocked(*state_);
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
        ClearCompletionFailureEntriesLocked(*state_);
        SetLastStatusLocked(*state_, ThreadWorkerStatus::NotStarted);
        return ThreadWorkerStatus::NotStarted;
    }

    if (state_->snapshot.is_shutdown) {
        ClearCompletionFailureEntriesLocked(*state_);
        SetLastStatusLocked(*state_, ThreadWorkerStatus::StopRequested);
        return ThreadWorkerStatus::StopRequested;
    }

    state_->snapshot.is_shutdown = true;
    state_->snapshot.shutdown_policy = policy;

    if (policy == ShutdownPolicy::CancelQueued) {
        CancelQueuedTasksLocked(*state_);
    }

    ClearCompletionFailureEntriesLocked(*state_);
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
            ClearCompletionFailureEntriesLocked(*state_);
            SetLastStatusLocked(*state_, ThreadWorkerStatus::ShutdownComplete);
            return ThreadWorkerStatus::ShutdownComplete;
        }

        if (!state_->snapshot.is_started) {
            ClearCompletionFailureEntriesLocked(*state_);
            SetLastStatusLocked(*state_, ThreadWorkerStatus::NotStarted);
            return ThreadWorkerStatus::NotStarted;
        }

        if (!state_->snapshot.is_shutdown) {
            ClearCompletionFailureEntriesLocked(*state_);
            SetLastStatusLocked(*state_, ThreadWorkerStatus::StopRequired);
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
        ClearCompletionFailureEntriesLocked(*state_);
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
        if (state_ != nullptr) {
            std::lock_guard<std::mutex> lock(state_->mutex);
            ClearCompletionFailureEntriesLocked(*state_);
            SetLastStatusLocked(*state_, ThreadWorkerStatus::InvalidArgument);
        }

        return ThreadWorkerStatus::InvalidArgument;
    }

    if (state_ == nullptr) {
        return ThreadWorkerStatus::NotInitialized;
    }

    if (output_capacity > 0U && output_records == nullptr) {
        std::lock_guard<std::mutex> lock(state_->mutex);
        ClearCompletionFailureEntriesLocked(*state_);
        SetLastStatusLocked(*state_, ThreadWorkerStatus::InvalidArgument);
        return ThreadWorkerStatus::InvalidArgument;
    }

    std::lock_guard<std::mutex> lock(state_->mutex);
    const ThreadWorkerStatus status_before_drain = state_->snapshot.last_status;
    const std::size_t required_completion_count = state_->snapshot.completion_pending_count;
    if (required_completion_count > output_capacity) {
        SetCompletionQueueFullLocked(*state_, required_completion_count);
        SetCompletionDrainOutputEntryLocked(*state_, output_capacity);
        return ThreadWorkerStatus::CompletionQueueFull;
    }

    std::size_t written_records = 0U;
    while (state_->snapshot.completion_pending_count > 0U && written_records < output_capacity) {
        output_records[written_records] = state_->completion_records[state_->completion_head];
        state_->completion_records[state_->completion_head] = ThreadWorkerCompletion{};
        state_->completion_head = (state_->completion_head + 1U) % state_->completion_records.size();
        --state_->snapshot.completion_pending_count;
        ++state_->snapshot.drained_completion_count;
        ++written_records;
    }

    *written_count = written_records;

    if (state_->snapshot.completion_pending_count > 0U) {
        SetLastStatusLocked(*state_, ThreadWorkerStatus::CompletionQueueFull);
        return ThreadWorkerStatus::CompletionQueueFull;
    }

    ClearCompletionDrainOutputEntryLocked(*state_);

    if (ShouldPreserveStatusAfterDrain(status_before_drain)) {
        ClearCompletionFailureEntriesLocked(*state_);
        SetLastStatusLocked(*state_, status_before_drain);
        return ThreadWorkerStatus::Success;
    }

    ClearCompletionFailureEntriesLocked(*state_);
    SetLastStatusLocked(*state_, ThreadWorkerStatus::Success);
    return ThreadWorkerStatus::Success;
}

ThreadWorkerCompletionLookupStatus ThreadWorker::LookupCompletion(
    TaskId task_id,
    ThreadWorkerCompletion* output_record) const {
    if (output_record == nullptr) {
        return ThreadWorkerCompletionLookupStatus::InvalidArgument;
    }

    if (task_id.value == INVALID_TASK_ID) {
        return ThreadWorkerCompletionLookupStatus::InvalidArgument;
    }

    if (state_ == nullptr) {
        return ThreadWorkerCompletionLookupStatus::NotInitialized;
    }

    std::lock_guard<std::mutex> lock(state_->mutex);
    ThreadWorkerCompletion lookup_record{};
    if (FindPendingWorkCompletionLocked(*state_, task_id, &lookup_record)) {
        *output_record = lookup_record;
        return ThreadWorkerCompletionLookupStatus::Success;
    }

    if (FindPendingCompletionLocked(*state_, task_id, &lookup_record)) {
        *output_record = lookup_record;
        return ThreadWorkerCompletionLookupStatus::Success;
    }

    if (FindCompletionSnapshotLocked(*state_, task_id, &lookup_record)) {
        *output_record = lookup_record;
        return ThreadWorkerCompletionLookupStatus::Success;
    }

    return ThreadWorkerCompletionLookupStatus::NotFound;
}

ThreadWorkerCompletionEnumerationResult ThreadWorker::EnumerateCompletionsByStatus(
    TaskStatus status,
    ThreadWorkerCompletion* output_records,
    std::size_t output_capacity) const {
    ThreadWorkerCompletionEnumerationResult result;
    if (!IsCompletionEnumerationStatus(status)) {
        result.status = ThreadWorkerCompletionEnumerationStatus::InvalidArgument;
        return result;
    }

    if (output_capacity > 0U && output_records == nullptr) {
        result.status = ThreadWorkerCompletionEnumerationStatus::InvalidArgument;
        return result;
    }

    if (state_ == nullptr) {
        result.status = ThreadWorkerCompletionEnumerationStatus::NotInitialized;
        return result;
    }

    std::lock_guard<std::mutex> lock(state_->mutex);
    const std::size_t required_count = CountCompletionSnapshotsByStatusLocked(*state_, status);
    result.required_count = required_count;
    if (required_count > output_capacity) {
        result.status = ThreadWorkerCompletionEnumerationStatus::OutputCapacityExceeded;
        return result;
    }

    WriteCompletionSnapshotsByStatusLocked(*state_, status, output_records);
    result.written_count = required_count;
    result.status = ThreadWorkerCompletionEnumerationStatus::Success;
    return result;
}

ThreadWorkerSnapshot ThreadWorker::Snapshot() const {
    if (state_ == nullptr) {
        return ThreadWorkerSnapshot{};
    }

    std::lock_guard<std::mutex> lock(state_->mutex);
    return state_->snapshot;
}
}
