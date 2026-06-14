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

    task_result_t Submit(TaskCallback callback, void* context);
    task_result_t Drain(InlineTaskExecutor& executor);
    task_result_t Shutdown(SHUTDOWN_POLICY policy, InlineTaskExecutor& executor);
    task_scheduler_snapshot_t Snapshot() const;
    std::size_t Capacity() const;

private:
    void CancelQueuedTasks();
    task_result_t RejectResult() const;
    task_result_t CompleteResult() const;

    std::vector<task_record_t> _records;
    memory::IMemoryTracker& _memoryTracker;
    task_scheduler_snapshot_t _snapshot;
    std::size_t _headIndex;
    std::size_t _tailIndex;
    std::uint64_t _nextTaskId;
};
}
