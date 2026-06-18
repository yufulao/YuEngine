// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/AsyncFileReadQueue.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/File/AsyncFileReadRequest.h"
#include "YuEngine/File/AsyncFileReadResult.h"
#include "YuEngine/File/AsyncFileReadStatus.h"

namespace yuengine::file {
struct AsyncFileReadQueueState;

struct AsyncFileReadQueueSnapshot {
    std::size_t work_capacity = 0U;
    std::size_t completion_capacity = 0U;
    std::uint64_t submitted_count = 0U;
    std::uint64_t completed_count = 0U;
    std::uint64_t failed_count = 0U;
    std::uint64_t canceled_count = 0U;
    std::uint64_t rejected_count = 0U;
    std::uint64_t drained_completion_count = 0U;
    std::uint64_t read_byte_count = 0U;
    std::size_t pending_count = 0U;
    std::size_t completion_pending_count = 0U;
    std::size_t max_queue_depth = 0U;
    std::size_t max_completion_depth = 0U;
    AsyncFileReadStatus last_status = AsyncFileReadStatus::NotInitialized;
    bool is_initialized = false;
    bool is_started = false;
    bool is_shutdown = false;
};

class AsyncFileReadQueue final {
public:
    /**
     * @comment 构造 AsyncFileReadQueue 所有者。
     */
    AsyncFileReadQueue();
    /**
     * @comment 在尽力显式 关闭 后销毁 AsyncFileReadQueue 所有者。
     */
    ~AsyncFileReadQueue();

    AsyncFileReadQueue(const AsyncFileReadQueue& other) = delete;
    AsyncFileReadQueue& operator=(const AsyncFileReadQueue& other) = delete;
    AsyncFileReadQueue(AsyncFileReadQueue&& other) = delete;
    AsyncFileReadQueue& operator=(AsyncFileReadQueue&& other) = delete;

    /**
     * @comment 初始化固定容量 async file queue 存储。
     * @param work_capacity 输入 工作容量。
     * @param completion_capacity 输入 完成容量。
     * @return 显式操作状态。
     */
    AsyncFileReadStatus Initialize(std::size_t work_capacity, std::size_t completion_capacity);
    /**
     * @comment 启动 私有 工作线程。
     * @return 显式操作状态。
     */
    AsyncFileReadStatus Start();
    /**
     * @comment 提交 an async file read 请求。
     * @param request 输入 请求。
     * @return 显式操作状态。
     */
    AsyncFileReadStatus Submit(const AsyncFileReadRequest& request);
    /**
     * @comment 请求 关闭。
     * @param cancel_pending true 表示取消等待任务，false 表示继续排空。
     * @return 显式操作状态。
     */
    AsyncFileReadStatus RequestStop(bool cancel_pending);
    /**
     * @comment 等待私有工作线程退出。
     * @return 显式操作状态。
     */
    AsyncFileReadStatus Join();
    /**
     * @comment 请求关闭并等待私有工作线程退出。
     * @param cancel_pending true 表示取消等待任务，false 表示继续排空。
     * @return 显式操作状态。
     */
    AsyncFileReadStatus Shutdown(bool cancel_pending);
    /**
     * @comment 提取 async file completion 记录 写入 调用方持有 存储.
     * @param output_results 输出 结果 存储。
     * @param output_capacity 输出 存储容量。
     * @param written_count 输出 写入数量。
     * @return 显式操作状态。
     */
    AsyncFileReadStatus DrainCompletions(
        AsyncFileReadResult* output_results,
        std::size_t output_capacity,
        std::size_t* written_count);
    /**
     * @comment 返回 async file queue 快照。
     * @return 快照值。
     */
    AsyncFileReadQueueSnapshot Snapshot() const;

private:
    AsyncFileReadQueueState* state_;
};
}
