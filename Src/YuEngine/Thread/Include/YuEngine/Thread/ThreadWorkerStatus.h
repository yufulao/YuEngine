// Module: YuEngine Thread
// File: Src/YuEngine/Thread/Include/YuEngine/Thread/ThreadWorkerStatus.h

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
