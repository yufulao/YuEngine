#pragma once

#include "yuengine/thread/task_id.h"
#include "yuengine/thread/task_status.h"

namespace yuengine::thread {
struct TaskResult {
    TaskId Id;
    TASK_STATUS Status;
};
}
