#pragma once

#include "YuEngine/Thread/TaskId.h"
#include "YuEngine/Thread/TaskStatus.h"

namespace yuengine::thread {
struct TaskResult {
    TaskId Id;
    TaskStatus Status;
};
}
