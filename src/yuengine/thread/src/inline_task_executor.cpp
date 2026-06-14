#include "yuengine/thread/inline_task_executor.h"

namespace yuengine::thread {
TaskStatus InlineTaskExecutor::Execute(TaskCallback callback, void* context) {
    if (callback == nullptr) {
        return TaskStatus::Failed;
    }

    return callback(context);
}
}
