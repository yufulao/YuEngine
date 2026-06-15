// Module: YuEngine Thread
// File: Src/YuEngine/Thread/Include/YuEngine/Thread/BoundedTaskQueue.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "YuEngine/Memory/IMemoryTracker.h"
#include "YuEngine/Thread/InlineTaskExecutor.h"
#include "YuEngine/Thread/ShutdownPolicy.h"
#include "YuEngine/Thread/TaskCallback.h"
#include "YuEngine/Thread/TaskRecord.h"
#include "YuEngine/Thread/TaskResult.h"
#include "YuEngine/Thread/TaskSchedulerSnapshot.h"

namespace yuengine::thread {
class BoundedTaskQueue final {
public:
    /**
     * @comment Constructs a BoundedTaskQueue instance.
     * @param capacity Input capacity.
     * @param memory_tracker Memory tracker updated by the function.
     */
    BoundedTaskQueue(std::size_t capacity, memory::IMemoryTracker& memory_tracker);

    /**
     * @comment Submits requested work.
     * @param callback Input callback.
     * @param context Input context.
     * @return Explicit operation result.
     */
    TaskResult Submit(TaskCallback callback, void* context);
    /**
     * @comment Drains queued work.
     * @param executor Executor updated by the function.
     * @return Explicit operation result.
     */
    TaskResult Drain(InlineTaskExecutor& executor);
    /**
     * @comment Shuts down the component.
     * @param policy Input policy.
     * @param executor Executor updated by the function.
     * @return Explicit operation result.
     */
    TaskResult Shutdown(ShutdownPolicy policy, InlineTaskExecutor& executor);
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    TaskSchedulerSnapshot Snapshot() const;
    /**
     * @comment Returns the storage capacity.
     * @return Capacity value.
     */
    std::size_t Capacity() const;

private:
    void CancelQueuedTasks();
    TaskResult RejectResult() const;
    TaskResult CompleteResult() const;

    std::vector<TaskRecord> records_;
    memory::IMemoryTracker& memory_tracker_;
    TaskSchedulerSnapshot snapshot_;
    std::size_t head_index_;
    std::size_t tail_index_;
    std::uint64_t next_task_id_;
};
}
