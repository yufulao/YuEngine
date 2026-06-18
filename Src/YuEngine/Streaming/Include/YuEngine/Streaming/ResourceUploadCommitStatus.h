// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadCommitStatus.h

#pragma once

namespace yuengine::streaming {
enum class ResourceUploadCommitStatus {
    Success,
    Queued,
    InvalidArgument,
    QueueFull,
    CompletionQueueFull,
    DuplicateCommitId,
    ResourceCommitFailed
};
}
