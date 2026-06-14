#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "yuengine/memory/i_memory_tracker.h"
#include "yuengine/thread/inline_task_executor.h"
#include "yuengine/thread/shutdown_policy.h"
#include "yuengine/thread/task_callback.h"
#include "yuengine/thread/task_record.h"
#include "yuengine/thread/task_result.h"
#include "yuengine/thread/task_scheduler_snapshot.h"

namespace yuengine::thread {
class BoundedTaskQueue final {
public:
    BoundedTaskQueue(std::size_t capacity, memory::IMemoryTracker& memoryTracker);

    TaskResult Submit(TaskCallback callback, void* context);
    TaskResult Drain(InlineTaskExecutor& executor);
    TaskResult Shutdown(ShutdownPolicy policy, InlineTaskExecutor& executor);
    TaskSchedulerSnapshot Snapshot() const;
    std::size_t Capacity() const;

private:
    void CancelQueuedTasks();
    TaskResult RejectResult() const;
    TaskResult CompleteResult() const;

    std::vector<TaskRecord> _records;
    memory::IMemoryTracker& _memoryTracker;
    TaskSchedulerSnapshot _snapshot;
    std::size_t _headIndex;
    std::size_t _tailIndex;
    std::uint64_t _nextTaskId;
};
}
