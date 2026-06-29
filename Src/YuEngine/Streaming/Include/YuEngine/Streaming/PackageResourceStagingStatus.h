// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/PackageResourceStagingStatus.h

#pragma once

namespace yuengine::streaming {
enum class PackageResourceStagingStatus {
    Success,
    Queued,
    InvalidArgument,
    InvalidPackageRecord,
    ResourceValidationFailed,
    TypeMismatch,
    ByteRangeOutOfBounds,
    OutputTooSmall,
    QueueFull,
    CompletionQueueFull,
    DuplicateRequestId,
    FileSubmitFailed,
    FileReadFailed,
    FileByteCountMismatch,
    MissingFileCompletion
};
}
