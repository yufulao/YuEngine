#pragma once

#include "yuengine/thread/task_callback.h"
#include "yuengine/thread/task_status.h"

namespace yuengine::thread {
class InlineTaskExecutor final {
public:
    TASK_STATUS Execute(TaskCallback callback, void* context);
};
}
