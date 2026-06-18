// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceStreamingPipelineStatus.h

#pragma once

namespace yuengine::streaming {
enum class ResourceStreamingPipelineStatus {
    Success,
    Queued,
    InvalidArgument,
    Busy,
    NotSubmitted,
    StagingSubmitFailed,
    FileCompletionFailed,
    StagingCompletionMissing,
    UploadSubmitFailed,
    UploadProcessFailed,
    UploadCompletionMissing,
    CommitSubmitFailed,
    CommitProcessFailed,
    CommitCompletionMissing
};
}
