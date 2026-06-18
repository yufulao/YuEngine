// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Include/YuEngine/Thread/BoundedTaskQueue.h

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
     * @comment 构造 BoundedTaskQueue 实例。
     * @param capacity 输入容量。
     * @param memory_tracker 函数更新的 memory tracker。
     */
    BoundedTaskQueue(std::size_t capacity, memory::IMemoryTracker& memory_tracker);

    /**
     * @comment 提交 requested work。
     * @param callback 输入 callback。
     * @param context 输入 context。
     * @return 显式操作结果。
     */
    TaskResult Submit(TaskCallback callback, void* context);
    /**
     * @comment 排空 queued work。
     * @param executor 函数更新的 executor。
     * @return 显式操作结果。
     */
    TaskResult Drain(InlineTaskExecutor& executor);
    /**
     * @comment 关闭 component。
     * @param policy 输入 policy。
     * @param executor 函数更新的 executor。
     * @return 显式操作结果。
     */
    TaskResult Shutdown(ShutdownPolicy policy, InlineTaskExecutor& executor);
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    TaskSchedulerSnapshot Snapshot() const;
    /**
     * @comment 返回 storage capacity。
     * @return Capacity 值。
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
