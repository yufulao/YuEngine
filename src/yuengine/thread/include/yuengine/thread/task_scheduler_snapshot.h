#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::thread {
struct TaskSchedulerSnapshot {
    std::uint64_t SubmittedCount;
    std::uint64_t ExecutedCount;
    std::uint64_t RejectedCount;
    std::uint64_t FailedCount;
    std::uint64_t CanceledCount;
    std::uint64_t DrainCount;
    std::uint64_t TaskExecutionAllocationCount;
    std::size_t MaxQueueDepth;
    std::size_t CapacityBeforeFixture;
    std::size_t CapacityAfterLastDrain;
    std::size_t PendingCount;
    bool IsShutdown;
};
}
