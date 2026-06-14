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
    BoundedTaskQueue(std::size_t capacity, memory::IMemoryTracker& memory_tracker);

    TaskResult Submit(TaskCallback callback, void* context);
    TaskResult Drain(InlineTaskExecutor& executor);
    TaskResult Shutdown(ShutdownPolicy policy, InlineTaskExecutor& executor);
    TaskSchedulerSnapshot Snapshot() const;
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
