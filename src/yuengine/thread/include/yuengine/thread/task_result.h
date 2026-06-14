#pragma once

#include "yuengine/thread/task_id.h"
#include "yuengine/thread/task_status.h"

namespace yuengine::thread {
struct task_result_t {
    task_id_t Id;
    TASK_STATUS Status;
};
}
