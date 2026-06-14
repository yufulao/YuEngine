#pragma once

#include "yuengine/thread/task_callback.h"
#include "yuengine/thread/task_id.h"
#include "yuengine/thread/task_status.h"

namespace yuengine::thread {
struct TaskRecord {
    TaskId Id;
    TaskCallback Callback;
    void* Context;
    TaskStatus Status;
};
}
