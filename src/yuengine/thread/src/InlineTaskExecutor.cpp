#include "yuengine/thread/InlineTaskExecutor.h"

namespace yuengine::thread {
TaskStatus InlineTaskExecutor::Execute(TaskCallback callback, void* context) {
    if (callback == nullptr) {
        return TaskStatus::Failed;
    }

    return callback(context);
}
}
