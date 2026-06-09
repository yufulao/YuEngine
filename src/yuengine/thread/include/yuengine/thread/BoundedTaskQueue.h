#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "yuengine/memory/IMemoryTracker.h"
#include "yuengine/thread/InlineTaskExecutor.h"
#include "yuengine/thread/ShutdownPolicy.h"
#include "yuengine/thread/TaskCallback.h"
#include "yuengine/thread/TaskRecord.h"
#include "yuengine/thread/TaskResult.h"
#include "yuengine/thread/TaskSchedulerSnapshot.h"

namespace yuengine::thread
{
class BoundedTaskQueue final
{
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
