#pragma once

#include "yuengine/thread/TaskCallback.h"
#include "yuengine/thread/TaskStatus.h"

namespace yuengine::thread
{
class InlineTaskExecutor final
{
public:
    TaskStatus Execute(TaskCallback callback, void* context);
};
}
