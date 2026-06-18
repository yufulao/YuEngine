// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Include/YuEngine/Thread/InlineTaskExecutor.h

#pragma once

#include "YuEngine/Thread/TaskCallback.h"
#include "YuEngine/Thread/TaskStatus.h"

namespace yuengine::thread {
class InlineTaskExecutor final {
public:
    /**
     * @comment 执行 queued work。
     * @param callback 输入 callback。
     * @param context 输入 context。
     * @return 显式操作状态。
     */
    TaskStatus Execute(TaskCallback callback, void* context);
};
}
