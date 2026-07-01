// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Include/YuEngine/Thread/TaskSchedulerSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Thread/TaskId.h"
#include "YuEngine/Thread/TaskStatus.h"

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
    TaskStatus last_status;
    bool is_shutdown;
    TaskId last_failed_task_id{0U};
    std::size_t last_required_queued_task_count = 0U;
    std::size_t last_failed_queue_capacity = 0U;
    std::size_t last_failed_queued_count = 0U;
    std::uint64_t last_failed_task_completed_count = 0U;
    std::uint64_t last_failed_task_failed_count = 0U;
    std::uint64_t last_failed_task_canceled_count = 0U;
};
}
