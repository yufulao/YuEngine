// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Include/YuEngine/Thread/TaskStatus.h

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
