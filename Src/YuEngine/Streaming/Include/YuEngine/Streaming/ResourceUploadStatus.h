// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadStatus.h

#pragma once

namespace yuengine::streaming {
enum class ResourceUploadStatus {
    Success,
    Queued,
    InvalidArgument,
    InvalidStagingCompletion,
    ResourceValidationFailed,
    TypeMismatch,
    ByteRangeOutOfBounds,
    EmptyUploadBytes,
    UnsupportedUploadKind,
    QueueFull,
    CompletionQueueFull,
    DuplicateUploadId,
    RhiUploadFailed
};
}
