// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Include/YuEngine/Thread/ThreadWorker.h

#pragma once

#include <cstddef>

#include "YuEngine/Thread/ShutdownPolicy.h"
#include "YuEngine/Thread/TaskCallback.h"
#include "YuEngine/Thread/TaskId.h"
#include "YuEngine/Thread/ThreadWorkerDesc.h"
#include "YuEngine/Thread/ThreadWorkerSnapshot.h"
#include "YuEngine/Thread/ThreadWorkerStatus.h"

namespace yuengine::thread {
struct ThreadWorkerState;

class ThreadWorker final {
public:
    /**
     * @comment 构造 ThreadWorker owner。
     */
    ThreadWorker();
    /**
     * @comment 在显式 shutdown best effort 后销毁 ThreadWorker owner。
     */
    ~ThreadWorker();

    ThreadWorker(const ThreadWorker& other) = delete;
    ThreadWorker& operator=(const ThreadWorker& other) = delete;
    ThreadWorker(ThreadWorker&& other) = delete;
    ThreadWorker& operator=(ThreadWorker&& other) = delete;

    /**
     * @comment 初始化 bounded worker storage。
     * @param desc 输入 descriptor。
     * @return 显式操作状态。
     */
    ThreadWorkerStatus Initialize(const ThreadWorkerDesc& desc);
    /**
     * @comment 启动 owned worker thread。
     * @return 显式操作状态。
     */
    ThreadWorkerStatus Start();
    /**
     * @comment 提交 bounded work。
     * @param callback 输入 callback。
     * @param context 输入 context。
     * @param task_id 输出 task id。
     * @return 显式操作状态。
     */
    ThreadWorkerStatus Submit(TaskCallback callback, void* context, TaskId* task_id=nullptr);
    /**
     * @comment 按提供的 policy 请求 worker stop。
     * @param policy 输入 shutdown policy。
     * @return 显式操作状态。
     */
    ThreadWorkerStatus RequestStop(ShutdownPolicy policy);
    /**
     * @comment 在请求 stop 后 join worker。
     * @return 显式操作状态。
     */
    ThreadWorkerStatus Join();
    /**
     * @comment 请求 stop 并 join worker。
     * @param policy 输入 shutdown policy。
     * @return 显式操作状态。
     */
    ThreadWorkerStatus Shutdown(ShutdownPolicy policy);
    /**
     * @comment 将 completion records 排空到调用方持有存储。
     * @param output_records 输出 completion records。
     * @param output_capacity 输出 completion capacity。
     * @param written_count 输出写入数量。
     * @return 显式操作状态。
     */
    ThreadWorkerStatus DrainCompletions(
        ThreadWorkerCompletion* output_records,
        std::size_t output_capacity,
        std::size_t* written_count);
    /**
     * @comment 返回 worker snapshot。
     * @return 快照值。
     */
    ThreadWorkerSnapshot Snapshot() const;

private:
    ThreadWorkerState* state_;
};
}
