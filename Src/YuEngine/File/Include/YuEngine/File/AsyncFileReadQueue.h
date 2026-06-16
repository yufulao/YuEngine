// Module: YuEngine File
// File: Src/YuEngine/File/Include/YuEngine/File/AsyncFileReadQueue.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/File/AsyncFileReadRequest.h"
#include "YuEngine/File/AsyncFileReadResult.h"
#include "YuEngine/File/AsyncFileReadStatus.h"

namespace yuengine::file {
struct AsyncFileReadQueueState;

struct AsyncFileReadQueueSnapshot {
    std::size_t work_capacity = 0U;
    std::size_t completion_capacity = 0U;
    std::uint64_t submitted_count = 0U;
    std::uint64_t completed_count = 0U;
    std::uint64_t failed_count = 0U;
    std::uint64_t canceled_count = 0U;
    std::uint64_t rejected_count = 0U;
    std::uint64_t drained_completion_count = 0U;
    std::uint64_t read_byte_count = 0U;
    std::size_t pending_count = 0U;
    std::size_t completion_pending_count = 0U;
    std::size_t max_queue_depth = 0U;
    std::size_t max_completion_depth = 0U;
    AsyncFileReadStatus last_status = AsyncFileReadStatus::NotInitialized;
    bool is_initialized = false;
    bool is_started = false;
    bool is_shutdown = false;
};

class AsyncFileReadQueue final {
public:
    /**
     * @comment Constructs an AsyncFileReadQueue owner.
     */
    AsyncFileReadQueue();
    /**
     * @comment Destroys an AsyncFileReadQueue owner after explicit shutdown best effort.
     */
    ~AsyncFileReadQueue();

    AsyncFileReadQueue(const AsyncFileReadQueue& other) = delete;
    AsyncFileReadQueue& operator=(const AsyncFileReadQueue& other) = delete;
    AsyncFileReadQueue(AsyncFileReadQueue&& other) = delete;
    AsyncFileReadQueue& operator=(AsyncFileReadQueue&& other) = delete;

    /**
     * @comment Initializes bounded async file queue storage.
     * @param work_capacity Input work capacity.
     * @param completion_capacity Input completion capacity.
     * @return Explicit operation status.
     */
    AsyncFileReadStatus Initialize(std::size_t work_capacity, std::size_t completion_capacity);
    /**
     * @comment Starts the private worker.
     * @return Explicit operation status.
     */
    AsyncFileReadStatus Start();
    /**
     * @comment Submits an async file read request.
     * @param request Input request.
     * @return Explicit operation status.
     */
    AsyncFileReadStatus Submit(const AsyncFileReadRequest& request);
    /**
     * @comment Requests shutdown.
     * @param cancel_pending True cancels pending work; false drains it.
     * @return Explicit operation status.
     */
    AsyncFileReadStatus RequestStop(bool cancel_pending);
    /**
     * @comment Joins the private worker.
     * @return Explicit operation status.
     */
    AsyncFileReadStatus Join();
    /**
     * @comment Requests shutdown and joins the private worker.
     * @param cancel_pending True cancels pending work; false drains it.
     * @return Explicit operation status.
     */
    AsyncFileReadStatus Shutdown(bool cancel_pending);
    /**
     * @comment Drains async file completion records into caller-owned storage.
     * @param output_results Output result storage.
     * @param output_capacity Output storage capacity.
     * @param written_count Output written count.
     * @return Explicit operation status.
     */
    AsyncFileReadStatus DrainCompletions(
        AsyncFileReadResult* output_results,
        std::size_t output_capacity,
        std::size_t* written_count);
    /**
     * @comment Returns an async file queue snapshot.
     * @return Snapshot value.
     */
    AsyncFileReadQueueSnapshot Snapshot() const;

private:
    AsyncFileReadQueueState* state_;
};
}
