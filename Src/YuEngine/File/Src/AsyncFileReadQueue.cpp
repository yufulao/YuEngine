// 模块: YuEngine File
// 文件: Src/YuEngine/File/Src/AsyncFileReadQueue.cpp

#include "YuEngine/File/AsyncFileReadQueue.h"

#include <algorithm>
#include <new>
#include <vector>

#include "YuEngine/File/FileReadResult.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/Thread/TaskStatus.h"
#include "YuEngine/Thread/ThreadWorker.h"

namespace yuengine::file {
namespace {
using ThreadWorker = yuengine::thread::ThreadWorker;
using ThreadWorkerCompletion = yuengine::thread::ThreadWorkerCompletion;
using ThreadWorkerDesc = yuengine::thread::ThreadWorkerDesc;
using ThreadWorkerSnapshot = yuengine::thread::ThreadWorkerSnapshot;
using ThreadWorkerStatus = yuengine::thread::ThreadWorkerStatus;
using yuengine::thread::ShutdownPolicy;
using yuengine::thread::TaskId;
using yuengine::thread::TaskStatus;

struct AsyncFileReadSlot {
    bool in_use = false;
    TaskId task_id{0U};
    AsyncFileReadRequest request;
    AsyncFileReadResult result;
};
}

struct AsyncFileReadQueueState final {
    std::vector<AsyncFileReadSlot> slots;
    std::vector<AsyncFileReadResult> completed_results;
    std::vector<ThreadWorkerCompletion> completion_buffer;
    ThreadWorker worker;
    AsyncFileReadQueueSnapshot snapshot;
    std::size_t completed_result_cursor = 0U;
};

namespace {
void ClearQueueCapacityFailure(AsyncFileReadQueueState& state) {
    state.snapshot.last_required_queue_capacity = 0U;
    state.snapshot.last_failed_request_index = 0U;
    state.snapshot.last_failed_read_request = FileReadRequest{};
    state.snapshot.last_failed_output_capacity = 0U;
    state.snapshot.last_failed_work_capacity = 0U;
    state.snapshot.last_failed_pending_count = 0U;
}

void SetLastStatus(AsyncFileReadQueueState& state, AsyncFileReadStatus status) {
    state.snapshot.last_status = status;
    state.snapshot.required_completion_output_count = 0U;
    ClearQueueCapacityFailure(state);
}

void RecordCompletionQueueFull(AsyncFileReadQueueState& state, std::size_t required_output_count) {
    SetLastStatus(state, AsyncFileReadStatus::CompletionQueueFull);
    state.snapshot.required_completion_output_count = required_output_count;
}

AsyncFileReadStatus MapWorkerStatus(ThreadWorkerStatus status) {
    if (status == ThreadWorkerStatus::Success) {
        return AsyncFileReadStatus::Success;
    }

    if (status == ThreadWorkerStatus::InvalidDescriptor) {
        return AsyncFileReadStatus::InvalidDescriptor;
    }

    if (status == ThreadWorkerStatus::InvalidArgument) {
        return AsyncFileReadStatus::InvalidArgument;
    }

    if (status == ThreadWorkerStatus::AllocationFailure) {
        return AsyncFileReadStatus::AllocationFailure;
    }

    if (status == ThreadWorkerStatus::AlreadyInitialized) {
        return AsyncFileReadStatus::AlreadyInitialized;
    }

    if (status == ThreadWorkerStatus::NotInitialized) {
        return AsyncFileReadStatus::NotInitialized;
    }

    if (status == ThreadWorkerStatus::AlreadyStarted) {
        return AsyncFileReadStatus::AlreadyStarted;
    }

    if (status == ThreadWorkerStatus::NotStarted) {
        return AsyncFileReadStatus::NotStarted;
    }

    if (status == ThreadWorkerStatus::QueueFull) {
        return AsyncFileReadStatus::QueueFull;
    }

    if (status == ThreadWorkerStatus::CompletionQueueFull) {
        return AsyncFileReadStatus::CompletionQueueFull;
    }

    if (status == ThreadWorkerStatus::StopRequested) {
        return AsyncFileReadStatus::ShutdownRequested;
    }

    if (status == ThreadWorkerStatus::ShutdownComplete) {
        return AsyncFileReadStatus::ShutdownComplete;
    }

    return AsyncFileReadStatus::WorkerFailure;
}

AsyncFileReadSlot* FindFreeSlot(AsyncFileReadQueueState& state) {
    for (AsyncFileReadSlot& slot : state.slots) {
        if (!slot.in_use) {
            return &slot;
        }
    }

    return nullptr;
}

AsyncFileReadSlot* FindSlotByTaskId(AsyncFileReadQueueState& state, TaskId task_id) {
    for (AsyncFileReadSlot& slot : state.slots) {
        if (!slot.in_use) {
            continue;
        }

        if (slot.task_id.value == task_id.value) {
            return &slot;
        }
    }

    return nullptr;
}

const AsyncFileReadResult* FindCompletedResultByRequestIndex(
    const AsyncFileReadQueueState& state,
    std::uint64_t request_index) {
    for (const AsyncFileReadResult& result : state.completed_results) {
        if (result.request_index == request_index) {
            return &result;
        }
    }

    return nullptr;
}

std::size_t CountUsedSlots(const AsyncFileReadQueueState& state) {
    std::size_t result = 0U;
    for (const AsyncFileReadSlot& slot : state.slots) {
        if (slot.in_use) {
            ++result;
        }
    }

    return result;
}

void ClearSlot(AsyncFileReadSlot& slot) {
    slot.in_use = false;
    slot.task_id = TaskId{0U};
    slot.request = AsyncFileReadRequest{};
    slot.result = AsyncFileReadResult{};
}

void RecordRejectedSubmit(AsyncFileReadQueueState& state, AsyncFileReadStatus status) {
    ++state.snapshot.rejected_count;
    SetLastStatus(state, status);
}

void RecordQueueCapacityFailure(
    AsyncFileReadQueueState& state,
    const AsyncFileReadRequest& request,
    std::size_t required_queue_capacity) {
    ++state.snapshot.rejected_count;
    SetLastStatus(state, AsyncFileReadStatus::QueueFull);
    state.snapshot.last_required_queue_capacity = required_queue_capacity;
    state.snapshot.last_failed_request_index = request.request_index;
    state.snapshot.last_failed_read_request = request.read_request;
    state.snapshot.last_failed_output_capacity = request.output_capacity;
    state.snapshot.last_failed_work_capacity = state.snapshot.work_capacity;
    state.snapshot.last_failed_pending_count = CountUsedSlots(state);
}

void StoreReadResult(AsyncFileReadSlot& slot, AsyncFileReadResult result) {
    slot.result = result;
}

void StoreCompletedResult(AsyncFileReadQueueState& state, const AsyncFileReadResult& result) {
    for (AsyncFileReadResult& completed_result : state.completed_results) {
        if (completed_result.request_index == result.request_index) {
            completed_result = result;
            return;
        }
    }

    if (state.completed_results.size() < state.slots.size()) {
        state.completed_results.emplace_back(result);
        return;
    }

    if (state.completed_results.empty()) {
        return;
    }

    state.completed_results[state.completed_result_cursor] = result;
    ++state.completed_result_cursor;
    if (state.completed_result_cursor >= state.completed_results.size()) {
        state.completed_result_cursor = 0U;
    }
}

TaskStatus ExecuteAsyncFileRead(void* context) {
    AsyncFileReadSlot* slot = static_cast<AsyncFileReadSlot*>(context);
    if (slot == nullptr) {
        return TaskStatus::Failed;
    }

    AsyncFileReadResult result;
    result.request_index = slot->request.request_index;

    if (slot->request.mount_table == nullptr) {
        result.status = AsyncFileReadStatus::InvalidArgument;
        result.file_status = FileStatus::ReadFailure;
        StoreReadResult(*slot, result);
        return TaskStatus::Failed;
    }

    FileReadResult read_result = slot->request.mount_table->Read(slot->request.read_request);
    result.file_status = read_result.status;

    if (!read_result.Succeeded()) {
        result.status = AsyncFileReadStatus::ReadFailure;
        StoreReadResult(*slot, result);
        return TaskStatus::Failed;
    }

    if (read_result.bytes.size() > slot->request.output_capacity) {
        result.status = AsyncFileReadStatus::OutputTooSmall;
        result.byte_count = read_result.bytes.size();
        StoreReadResult(*slot, result);
        return TaskStatus::Failed;
    }

    if (!read_result.bytes.empty() && slot->request.output_bytes == nullptr) {
        result.status = AsyncFileReadStatus::OutputTooSmall;
        result.byte_count = read_result.bytes.size();
        StoreReadResult(*slot, result);
        return TaskStatus::Failed;
    }

    if (!read_result.bytes.empty()) {
        std::copy(read_result.bytes.begin(), read_result.bytes.end(), slot->request.output_bytes);
    }

    result.status = AsyncFileReadStatus::Success;
    result.byte_count = read_result.bytes.size();
    StoreReadResult(*slot, result);
    return TaskStatus::Completed;
}

AsyncFileReadResult BuildCanceledResult(const AsyncFileReadSlot& slot) {
    AsyncFileReadResult result;
    result.status = AsyncFileReadStatus::Canceled;
    result.file_status = FileStatus::Success;
    result.request_index = slot.request.request_index;
    return result;
}

AsyncFileReadResult BuildWorkerFailureResult(const ThreadWorkerCompletion& completion) {
    AsyncFileReadResult result;
    result.status = AsyncFileReadStatus::WorkerFailure;
    result.file_status = FileStatus::ReadFailure;
    result.request_index = completion.task_id.value;
    return result;
}

void RecordDrainedResult(AsyncFileReadQueueState& state, const AsyncFileReadResult& result) {
    ++state.snapshot.drained_completion_count;

    if (result.status == AsyncFileReadStatus::Success) {
        ++state.snapshot.completed_count;
        state.snapshot.read_byte_count += result.byte_count;
    }

    if (result.status == AsyncFileReadStatus::ReadFailure) {
        ++state.snapshot.failed_count;
    }

    if (result.status == AsyncFileReadStatus::OutputTooSmall) {
        ++state.snapshot.failed_count;
    }

    if (result.status == AsyncFileReadStatus::WorkerFailure) {
        ++state.snapshot.failed_count;
    }

    if (result.status == AsyncFileReadStatus::Canceled) {
        ++state.snapshot.canceled_count;
    }
}

AsyncFileReadStatus SelectDrainedLastStatus(
    AsyncFileReadStatus current_status,
    const AsyncFileReadResult& result) {
    if (result.status == AsyncFileReadStatus::ReadFailure) {
        return AsyncFileReadStatus::ReadFailure;
    }

    if (result.status == AsyncFileReadStatus::OutputTooSmall) {
        return AsyncFileReadStatus::OutputTooSmall;
    }

    if (result.status == AsyncFileReadStatus::WorkerFailure) {
        return AsyncFileReadStatus::WorkerFailure;
    }

    if (result.status == AsyncFileReadStatus::Canceled) {
        return AsyncFileReadStatus::Canceled;
    }

    return current_status;
}
}

AsyncFileReadQueue::AsyncFileReadQueue()
    : state_(nullptr) {
}

AsyncFileReadQueue::~AsyncFileReadQueue() {
    if (state_ == nullptr) {
        return;
    }

    Shutdown(false);
    delete state_;
    state_ = nullptr;
}

AsyncFileReadStatus AsyncFileReadQueue::Initialize(std::size_t work_capacity, std::size_t completion_capacity) {
    if (state_ != nullptr) {
        return AsyncFileReadStatus::AlreadyInitialized;
    }

    if (work_capacity == 0U) {
        return AsyncFileReadStatus::InvalidDescriptor;
    }

    if (completion_capacity == 0U) {
        return AsyncFileReadStatus::InvalidDescriptor;
    }

    state_ = new (std::nothrow) AsyncFileReadQueueState();
    if (state_ == nullptr) {
        return AsyncFileReadStatus::AllocationFailure;
    }

    state_->slots.resize(work_capacity);
    state_->completed_results.reserve(work_capacity);
    state_->completion_buffer.resize(completion_capacity);

    ThreadWorkerDesc desc;
    desc.work_capacity = work_capacity;
    desc.completion_capacity = completion_capacity;
    desc.default_shutdown_policy = ShutdownPolicy::DrainQueued;

    const ThreadWorkerStatus worker_status = state_->worker.Initialize(desc);
    const AsyncFileReadStatus status = MapWorkerStatus(worker_status);
    if (status != AsyncFileReadStatus::Success) {
        delete state_;
        state_ = nullptr;
        return status;
    }

    state_->snapshot.work_capacity = work_capacity;
    state_->snapshot.completion_capacity = completion_capacity;
    state_->snapshot.is_initialized = true;
    SetLastStatus(*state_, AsyncFileReadStatus::Success);
    return AsyncFileReadStatus::Success;
}

AsyncFileReadStatus AsyncFileReadQueue::Start() {
    if (state_ == nullptr) {
        return AsyncFileReadStatus::NotInitialized;
    }

    const ThreadWorkerStatus worker_status = state_->worker.Start();
    const AsyncFileReadStatus status = MapWorkerStatus(worker_status);
    if (status == AsyncFileReadStatus::Success) {
        state_->snapshot.is_started = true;
    }

    SetLastStatus(*state_, status);
    return status;
}

AsyncFileReadStatus AsyncFileReadQueue::Submit(const AsyncFileReadRequest& request) {
    if (state_ == nullptr) {
        return AsyncFileReadStatus::NotInitialized;
    }

    if (request.mount_table == nullptr) {
        RecordRejectedSubmit(*state_, AsyncFileReadStatus::InvalidArgument);
        return AsyncFileReadStatus::InvalidArgument;
    }

    if (request.output_capacity > 0U && request.output_bytes == nullptr) {
        RecordRejectedSubmit(*state_, AsyncFileReadStatus::InvalidArgument);
        return AsyncFileReadStatus::InvalidArgument;
    }

    AsyncFileReadSlot* slot = FindFreeSlot(*state_);
    if (slot == nullptr) {
        const std::size_t required_queue_capacity = state_->slots.size() + 1U;
        RecordQueueCapacityFailure(*state_, request, required_queue_capacity);
        return AsyncFileReadStatus::QueueFull;
    }

    slot->in_use = true;
    slot->request = request;
    slot->result = AsyncFileReadResult{};
    slot->result.status = AsyncFileReadStatus::Queued;
    slot->result.request_index = request.request_index;

    const ThreadWorkerStatus worker_status = state_->worker.Submit(&ExecuteAsyncFileRead, slot, &slot->task_id);
    const AsyncFileReadStatus status = MapWorkerStatus(worker_status);
    if (status != AsyncFileReadStatus::Success) {
        ClearSlot(*slot);
        RecordRejectedSubmit(*state_, status);
        return status;
    }

    ++state_->snapshot.submitted_count;
    state_->snapshot.pending_count = CountUsedSlots(*state_);
    if (state_->snapshot.pending_count > state_->snapshot.max_queue_depth) {
        state_->snapshot.max_queue_depth = state_->snapshot.pending_count;
    }

    SetLastStatus(*state_, AsyncFileReadStatus::Queued);
    return AsyncFileReadStatus::Queued;
}

AsyncFileReadStatus AsyncFileReadQueue::RequestStop(bool cancel_pending) {
    if (state_ == nullptr) {
        return AsyncFileReadStatus::NotInitialized;
    }

    ShutdownPolicy policy = ShutdownPolicy::DrainQueued;
    if (cancel_pending) {
        policy = ShutdownPolicy::CancelQueued;
    }

    const ThreadWorkerStatus worker_status = state_->worker.RequestStop(policy);
    const AsyncFileReadStatus status = MapWorkerStatus(worker_status);
    if (status == AsyncFileReadStatus::Success) {
        state_->snapshot.is_shutdown = true;
    }

    SetLastStatus(*state_, status);
    return status;
}

AsyncFileReadStatus AsyncFileReadQueue::Join() {
    if (state_ == nullptr) {
        return AsyncFileReadStatus::NotInitialized;
    }

    const ThreadWorkerStatus worker_status = state_->worker.Join();
    const AsyncFileReadStatus status = MapWorkerStatus(worker_status);
    if (status == AsyncFileReadStatus::ShutdownComplete) {
        state_->snapshot.is_started = false;
        state_->snapshot.is_shutdown = true;
    }

    SetLastStatus(*state_, status);
    return status;
}

AsyncFileReadStatus AsyncFileReadQueue::Shutdown(bool cancel_pending) {
    AsyncFileReadStatus stop_status = RequestStop(cancel_pending);
    if (stop_status == AsyncFileReadStatus::NotInitialized) {
        return stop_status;
    }

    if (stop_status == AsyncFileReadStatus::NotStarted) {
        return stop_status;
    }

    AsyncFileReadStatus join_status = Join();
    return join_status;
}

AsyncFileReadStatus AsyncFileReadQueue::DrainCompletions(
    AsyncFileReadResult* output_results,
    std::size_t output_capacity,
    std::size_t* written_count) {
    if (written_count == nullptr) {
        if (state_ != nullptr) {
            SetLastStatus(*state_, AsyncFileReadStatus::InvalidArgument);
        }

        return AsyncFileReadStatus::InvalidArgument;
    }

    *written_count = 0U;

    if (state_ == nullptr) {
        return AsyncFileReadStatus::NotInitialized;
    }

    if (output_capacity > 0U && output_results == nullptr) {
        SetLastStatus(*state_, AsyncFileReadStatus::InvalidArgument);
        return AsyncFileReadStatus::InvalidArgument;
    }

    const ThreadWorkerSnapshot worker_snapshot = state_->worker.Snapshot();
    if (worker_snapshot.completion_pending_count > output_capacity) {
        RecordCompletionQueueFull(*state_, worker_snapshot.completion_pending_count);
        return AsyncFileReadStatus::CompletionQueueFull;
    }

    std::size_t worker_output_capacity = state_->completion_buffer.size();
    if (worker_output_capacity > output_capacity) {
        worker_output_capacity = output_capacity;
    }

    std::size_t worker_written_count = 0U;
    AsyncFileReadStatus drained_last_status = AsyncFileReadStatus::Success;
    const ThreadWorkerStatus worker_status = state_->worker.DrainCompletions(
        state_->completion_buffer.data(),
        worker_output_capacity,
        &worker_written_count);
    if (worker_status != ThreadWorkerStatus::Success) {
        if (worker_status == ThreadWorkerStatus::CompletionQueueFull) {
            const ThreadWorkerSnapshot full_snapshot = state_->worker.Snapshot();
            RecordCompletionQueueFull(*state_, full_snapshot.last_required_completion_count);
            return AsyncFileReadStatus::CompletionQueueFull;
        }

        SetLastStatus(*state_, MapWorkerStatus(worker_status));
        return MapWorkerStatus(worker_status);
    }

    for (std::size_t index = 0U; index < worker_written_count; ++index) {
        const ThreadWorkerCompletion worker_completion = state_->completion_buffer[index];
        state_->completion_buffer[index] = ThreadWorkerCompletion{};
        AsyncFileReadSlot* slot = FindSlotByTaskId(*state_, worker_completion.task_id);
        AsyncFileReadResult result = BuildWorkerFailureResult(worker_completion);
        if (slot != nullptr) {
            result = slot->result;
            if (worker_completion.status == TaskStatus::Canceled) {
                result = BuildCanceledResult(*slot);
            }

            ClearSlot(*slot);
        }

        StoreCompletedResult(*state_, result);
        output_results[*written_count] = result;
        ++(*written_count);
        RecordDrainedResult(*state_, result);
        drained_last_status = SelectDrainedLastStatus(drained_last_status, result);
    }

    const ThreadWorkerSnapshot final_worker_snapshot = state_->worker.Snapshot();
    state_->snapshot.pending_count = CountUsedSlots(*state_);
    state_->snapshot.completion_pending_count = final_worker_snapshot.completion_pending_count;
    if (final_worker_snapshot.max_completion_depth > state_->snapshot.max_completion_depth) {
        state_->snapshot.max_completion_depth = final_worker_snapshot.max_completion_depth;
    }

    if (final_worker_snapshot.completion_pending_count > 0U) {
        const std::size_t required_output_count =
            *written_count + final_worker_snapshot.completion_pending_count;
        RecordCompletionQueueFull(*state_, required_output_count);
        return AsyncFileReadStatus::CompletionQueueFull;
    }

    SetLastStatus(*state_, drained_last_status);
    return AsyncFileReadStatus::Success;
}

AsyncFileReadStatus AsyncFileReadQueue::GetCompletedResult(
    std::uint64_t request_index,
    AsyncFileReadResult* output_result) const {
    if (output_result == nullptr) {
        return AsyncFileReadStatus::InvalidArgument;
    }

    *output_result = AsyncFileReadResult{};

    if (state_ == nullptr) {
        return AsyncFileReadStatus::NotInitialized;
    }

    const AsyncFileReadResult* completed_result = FindCompletedResultByRequestIndex(*state_, request_index);
    if (completed_result == nullptr) {
        return AsyncFileReadStatus::InvalidArgument;
    }

    *output_result = *completed_result;
    return completed_result->status;
}

AsyncFileReadQueueSnapshot AsyncFileReadQueue::Snapshot() const {
    if (state_ == nullptr) {
        return AsyncFileReadQueueSnapshot{};
    }

    AsyncFileReadQueueSnapshot snapshot = state_->snapshot;
    const ThreadWorkerSnapshot worker_snapshot = state_->worker.Snapshot();
    snapshot.pending_count = CountUsedSlots(*state_);
    snapshot.completion_pending_count = worker_snapshot.completion_pending_count;
    snapshot.is_started = worker_snapshot.is_started;
    snapshot.is_shutdown = worker_snapshot.is_shutdown;

    if (worker_snapshot.max_queue_depth > snapshot.max_queue_depth) {
        snapshot.max_queue_depth = worker_snapshot.max_queue_depth;
    }

    if (worker_snapshot.max_completion_depth > snapshot.max_completion_depth) {
        snapshot.max_completion_depth = worker_snapshot.max_completion_depth;
    }

    return snapshot;
}
}
