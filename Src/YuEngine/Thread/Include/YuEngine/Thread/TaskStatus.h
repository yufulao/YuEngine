// Module: YuEngine Thread
// File: Src/YuEngine/Thread/Include/YuEngine/Thread/TaskStatus.h

#pragma once

namespace yuengine::thread {
enum class TaskStatus {
    Created,
    Queued,
    Running,
    Completed,
    Failed,
    Canceled,
    Rejected
};
}
