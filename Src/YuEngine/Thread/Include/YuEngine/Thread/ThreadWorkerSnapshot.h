// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Include/YuEngine/Thread/ThreadWorkerSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Thread/ShutdownPolicy.h"
#include "YuEngine/Thread/TaskId.h"
#include "YuEngine/Thread/TaskStatus.h"
#include "YuEngine/Thread/ThreadWorkerStatus.h"

namespace yuengine::thread {
struct ThreadWorkerCompletion {
    TaskId task_id{0U};
    TaskStatus status = TaskStatus::Created;
};

struct ThreadWorkerSnapshot {
    std::size_t work_capacity = 0U;
    std::size_t completion_capacity = 0U;
    std::size_t work_capacity_after_shutdown = 0U;
    std::size_t completion_capacity_after_shutdown = 0U;
    std::uint64_t submitted_count = 0U;
    std::uint64_t started_count = 0U;
    std::uint64_t completed_count = 0U;
    std::uint64_t failed_count = 0U;
    std::uint64_t canceled_count = 0U;
    std::uint64_t rejected_count = 0U;
    std::uint64_t drained_completion_count = 0U;
    std::size_t pending_count = 0U;
    std::size_t running_count = 0U;
    std::size_t completion_pending_count = 0U;
    std::size_t max_queue_depth = 0U;
    std::size_t max_completion_depth = 0U;
    std::size_t last_required_completion_count = 0U;
    TaskId last_failed_completion_task_id{0U};
    TaskStatus last_failed_completion_status = TaskStatus::Created;
    std::size_t last_failed_completion_queue_capacity = 0U;
    std::size_t last_failed_completion_pending_count = 0U;
    ShutdownPolicy shutdown_policy = ShutdownPolicy::DrainQueued;
    ThreadWorkerStatus last_status = ThreadWorkerStatus::NotInitialized;
    bool is_initialized = false;
    bool is_started = false;
    bool is_shutdown = false;
    bool is_joined = false;
};
}
