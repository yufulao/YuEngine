// Module: YuEngine Thread
// File: Src/YuEngine/Thread/Include/YuEngine/Thread/TaskRecord.h

#pragma once

#include "YuEngine/Thread/TaskCallback.h"
#include "YuEngine/Thread/TaskId.h"
#include "YuEngine/Thread/TaskStatus.h"

namespace yuengine::thread {
struct TaskRecord {
    TaskId id;
    TaskCallback callback;
    void* context;
    TaskStatus status;
};
}
