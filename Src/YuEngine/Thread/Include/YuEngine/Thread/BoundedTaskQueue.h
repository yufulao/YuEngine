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
