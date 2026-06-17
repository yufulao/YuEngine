// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/PackageResourceStagingStatus.h

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
    MissingFileCompletion
};
}
