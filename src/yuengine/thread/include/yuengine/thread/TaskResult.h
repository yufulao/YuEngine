#pragma once

#include "yuengine/thread/TaskId.h"
#include "yuengine/thread/TaskStatus.h"

namespace yuengine::thread
{
struct TaskResult
{
    TaskId Id;
    TaskStatus Status;
};
}
