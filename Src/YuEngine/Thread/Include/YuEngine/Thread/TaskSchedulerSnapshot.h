#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::thread {
struct TaskSchedulerSnapshot {
    std::uint64_t submitted_count;
    std::uint64_t executed_count;
    std::uint64_t rejected_count;
    std::uint64_t failed_count;
    std::uint64_t canceled_count;
    std::uint64_t drain_count;
    std::uint64_t task_execution_allocation_count;
    std::size_t max_queue_depth;
    std::size_t capacity_before_fixture;
    std::size_t capacity_after_last_drain;
    std::size_t pending_count;
    bool is_shutdown;
};
}
