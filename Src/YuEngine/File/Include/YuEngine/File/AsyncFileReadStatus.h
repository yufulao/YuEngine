// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/AsyncFileReadStatus.h

#pragma once

namespace yuengine::file {
enum class AsyncFileReadStatus {
    Success,
    Queued,
    Canceled,
    ReadFailure,
    OutputTooSmall,
    InvalidDescriptor,
    InvalidArgument,
    AllocationFailure,
    AlreadyInitialized,
    NotInitialized,
    AlreadyStarted,
    NotStarted,
    QueueFull,
    CompletionQueueFull,
    ShutdownRequested,
    WorkerFailure,
    ShutdownComplete
};
}
