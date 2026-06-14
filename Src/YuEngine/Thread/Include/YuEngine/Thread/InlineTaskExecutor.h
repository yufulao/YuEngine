#pragma once

#include "YuEngine/Thread/TaskCallback.h"
#include "YuEngine/Thread/TaskStatus.h"

namespace yuengine::thread {
class InlineTaskExecutor final {
public:
    TaskStatus Execute(TaskCallback callback, void* context);
};
}
