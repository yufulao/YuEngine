// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Src/InlineTaskExecutor.cpp

#include "YuEngine/Thread/InlineTaskExecutor.h"

namespace yuengine::thread {
TaskStatus InlineTaskExecutor::Execute(TaskCallback callback, void* context) {
    if (callback == nullptr) {
        return TaskStatus::Failed;
    }

    return callback(context);
}
}
