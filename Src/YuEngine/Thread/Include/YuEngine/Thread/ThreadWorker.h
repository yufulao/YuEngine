// Module: YuEngine Thread
// File: Src/YuEngine/Thread/Include/YuEngine/Thread/ThreadWorker.h

#pragma once

#include <cstddef>

#include "YuEngine/Thread/ShutdownPolicy.h"
#include "YuEngine/Thread/TaskCallback.h"
#include "YuEngine/Thread/TaskId.h"
#include "YuEngine/Thread/ThreadWorkerDesc.h"
#include "YuEngine/Thread/ThreadWorkerSnapshot.h"
#include "YuEngine/Thread/ThreadWorkerStatus.h"

namespace yuengine::thread {
struct ThreadWorkerState;

class ThreadWorker final {
public:
    /**
     * @comment Constructs a ThreadWorker owner.
     */
    ThreadWorker();
    /**
     * @comment Destroys a ThreadWorker owner after explicit shutdown best effort.
     */
    ~ThreadWorker();

    ThreadWorker(const ThreadWorker& other) = delete;
    ThreadWorker& operator=(const ThreadWorker& other) = delete;
    ThreadWorker(ThreadWorker&& other) = delete;
    ThreadWorker& operator=(ThreadWorker&& other) = delete;

    /**
     * @comment Initializes bounded worker storage.
     * @param desc Input descriptor.
     * @return Explicit operation status.
     */
    ThreadWorkerStatus Initialize(const ThreadWorkerDesc& desc);
    /**
     * @comment Starts the owned worker thread.
     * @return Explicit operation status.
     */
    ThreadWorkerStatus Start();
    /**
     * @comment Submits bounded work.
     * @param callback Input callback.
     * @param context Input context.
     * @param task_id Output task id.
     * @return Explicit operation status.
     */
    ThreadWorkerStatus Submit(TaskCallback callback, void* context, TaskId* task_id=nullptr);
    /**
     * @comment Requests worker stop using the provided policy.
     * @param policy Input shutdown policy.
     * @return Explicit operation status.
     */
    ThreadWorkerStatus RequestStop(ShutdownPolicy policy);
    /**
     * @comment Joins the worker after stop has been requested.
     * @return Explicit operation status.
     */
    ThreadWorkerStatus Join();
    /**
     * @comment Requests stop and joins the worker.
     * @param policy Input shutdown policy.
     * @return Explicit operation status.
     */
    ThreadWorkerStatus Shutdown(ShutdownPolicy policy);
    /**
     * @comment Drains completion records into caller-owned storage.
     * @param output_records Output completion records.
     * @param output_capacity Output completion capacity.
     * @param written_count Output written count.
     * @return Explicit operation status.
     */
    ThreadWorkerStatus DrainCompletions(
        ThreadWorkerCompletion* output_records,
        std::size_t output_capacity,
        std::size_t* written_count);
    /**
     * @comment Returns a worker snapshot.
     * @return Snapshot value.
     */
    ThreadWorkerSnapshot Snapshot() const;

private:
    ThreadWorkerState* state_;
};
}
