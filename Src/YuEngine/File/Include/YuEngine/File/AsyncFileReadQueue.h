// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/AsyncFileReadQueue.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/File/AsyncFileReadQueueSnapshot.h"
#include "YuEngine/File/AsyncFileReadRequest.h"
#include "YuEngine/File/AsyncFileReadResult.h"
#include "YuEngine/File/AsyncFileReadStatus.h"

namespace yuengine::file {
struct AsyncFileReadQueueState;

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
     * @comment 按 request_index 返回已排空 request 的最终结果。
     * @param request_index 输入 request 标识。
     * @param output_result 输出 request 最终结果。
     * @return request 最终状态；未找到时返回 InvalidArgument。
     */
    AsyncFileReadStatus GetCompletedResult(
        std::uint64_t request_index,
        AsyncFileReadResult* output_result) const;
    /**
     * @comment 返回 async file queue 快照。
     * @return 快照值。
     */
    AsyncFileReadQueueSnapshot Snapshot() const;

private:
    AsyncFileReadQueueState* state_;
};
}
