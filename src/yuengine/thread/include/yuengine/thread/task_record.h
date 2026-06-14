#pragma once

#include "yuengine/thread/task_callback.h"
#include "yuengine/thread/task_id.h"
#include "yuengine/thread/task_status.h"

namespace yuengine::thread {
struct task_record_t {
    task_id_t Id;
    TaskCallback Callback;
    void* Context;
    TASK_STATUS Status;
};
}
