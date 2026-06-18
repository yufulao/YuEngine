// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Include/YuEngine/Thread/TaskResult.h

#pragma once

#include "YuEngine/Thread/TaskId.h"
#include "YuEngine/Thread/TaskStatus.h"

namespace yuengine::thread {
struct TaskResult {
    TaskId id;
    TaskStatus status;
};
}
