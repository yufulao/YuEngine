#include "yuengine/thread/inline_task_executor.h"

namespace yuengine::thread {
TASK_STATUS InlineTaskExecutor::Execute(TaskCallback callback, void* context) {
    if (callback == nullptr) {
        return TASK_STATUS::Failed;
    }

    return callback(context);
}
}
