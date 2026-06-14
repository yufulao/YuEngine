#pragma once

#include "yuengine/thread/TaskCallback.h"
#include "yuengine/thread/TaskId.h"
#include "yuengine/thread/TaskStatus.h"

namespace yuengine::thread {
struct TaskRecord {
    TaskId Id;
    TaskCallback Callback;
    void* Context;
    TaskStatus Status;
};
}
