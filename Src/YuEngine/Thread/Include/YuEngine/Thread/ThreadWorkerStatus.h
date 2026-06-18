// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Include/YuEngine/Thread/ThreadWorkerStatus.h

#pragma once

namespace yuengine::thread {
enum class ThreadWorkerStatus {
    Success,
    InvalidDescriptor,
    InvalidArgument,
    AllocationFailure,
    AlreadyInitialized,
    NotInitialized,
    AlreadyStarted,
    NotStarted,
    StopRequired,
    StopRequested,
    QueueFull,
    CompletionQueueFull,
    ShutdownComplete
};
}
