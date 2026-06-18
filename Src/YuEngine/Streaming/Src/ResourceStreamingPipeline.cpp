// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Src/ResourceStreamingPipeline.cpp

#include "YuEngine/Streaming/ResourceStreamingPipeline.h"

#include <array>

namespace yuengine::streaming {
ResourceStreamingPipeline::ResourceStreamingPipeline()
    : staging_queue_(),
      upload_queue_(),
      commit_queue_(),
      request_(),
      last_staging_completion_(),
      last_upload_completion_(),
      last_commit_completion_(),
      snapshot_(),
      has_active_request_(false) {
}

ResourceStreamingPipelineStatus ResourceStreamingPipeline::Submit(
    const ResourceStreamingPipelineRequest &request) {
    if (has_active_request_) {
        return RecordRejected(ResourceStreamingPipelineStatus::Busy);
    }

    const ResourceStreamingPipelineStatus request_status = ValidateRequest(request);
    if (request_status != ResourceStreamingPipelineStatus::Success) {
        return RecordRejected(request_status);
    }

    const PackageResourceStagingRequest staging_request = BuildStagingRequest(request);
    const PackageResourceStagingStatus staging_status = staging_queue_.Submit(staging_request);
    snapshot_.last_staging_status = staging_status;
    if (staging_status != PackageResourceStagingStatus::Queued) {
        return RecordFailed(ResourceStreamingPipelineStatus::StagingSubmitFailed);
    }

    request_ = request;
    last_staging_completion_ = PackageResourceStagingCompletion{};
    last_upload_completion_ = ResourceUploadCompletion{};
    last_commit_completion_ = ResourceUploadCommitCompletion{};
    has_active_request_ = true;
    snapshot_.has_active_request = true;
    ++snapshot_.submitted_count;
    snapshot_.last_status = ResourceStreamingPipelineStatus::Queued;
    return ResourceStreamingPipelineStatus::Queued;
}

ResourceStreamingPipelineStatus ResourceStreamingPipeline::CompleteFileRead(
    const file::AsyncFileReadResult &file_result) {
    if (!has_active_request_) {
        return RecordRejected(ResourceStreamingPipelineStatus::NotSubmitted);
    }

    const PackageResourceStagingStatus staging_status = staging_queue_.CompleteFileRead(file_result);
    snapshot_.last_staging_status = staging_status;
    snapshot_.last_async_file_status = file_result.status;

    std::array<PackageResourceStagingCompletion, 1U> completions{};
    std::uint32_t written_count = 0U;
    const PackageResourceStagingStatus drain_status = staging_queue_.DrainCompletions(
        completions.data(),
        static_cast<std::uint32_t>(completions.size()),
        &written_count);
    snapshot_.last_staging_status = drain_status;
    if (drain_status != PackageResourceStagingStatus::Success) {
        return RecordFailed(ResourceStreamingPipelineStatus::StagingCompletionMissing);
    }

    if (written_count != 1U) {
        return RecordFailed(ResourceStreamingPipelineStatus::StagingCompletionMissing);
    }

    last_staging_completion_ = completions[0U];
    if (staging_status != PackageResourceStagingStatus::Success) {
        return RecordFailed(ResourceStreamingPipelineStatus::FileCompletionFailed);
    }

    const ResourceUploadRequest upload_request = BuildUploadRequest(last_staging_completion_);
    const ResourceUploadStatus upload_status = upload_queue_.Submit(upload_request);
    snapshot_.last_upload_status = upload_status;
    if (upload_status != ResourceUploadStatus::Queued) {
        return RecordFailed(ResourceStreamingPipelineStatus::UploadSubmitFailed);
    }

    snapshot_.last_status = ResourceStreamingPipelineStatus::Queued;
    return ResourceStreamingPipelineStatus::Queued;
}

ResourceStreamingPipelineStatus ResourceStreamingPipeline::ProcessUpload() {
    if (!has_active_request_) {
        return RecordRejected(ResourceStreamingPipelineStatus::NotSubmitted);
    }

    const ResourceUploadStatus upload_status = upload_queue_.ProcessNext();
    snapshot_.last_upload_status = upload_status;

    std::array<ResourceUploadCompletion, 1U> completions{};
    std::uint32_t written_count = 0U;
    const ResourceUploadStatus drain_status = upload_queue_.DrainCompletions(
        completions.data(),
        static_cast<std::uint32_t>(completions.size()),
        &written_count);
    if (drain_status != ResourceUploadStatus::Success) {
        snapshot_.last_upload_status = drain_status;
        return RecordFailed(ResourceStreamingPipelineStatus::UploadCompletionMissing);
    }

    if (written_count != 1U) {
        return RecordFailed(ResourceStreamingPipelineStatus::UploadCompletionMissing);
    }

    last_upload_completion_ = completions[0U];
    if (upload_status != ResourceUploadStatus::Success && upload_status != ResourceUploadStatus::RhiUploadFailed) {
        return RecordFailed(ResourceStreamingPipelineStatus::UploadProcessFailed);
    }

    const ResourceUploadCommitRequest commit_request = BuildCommitRequest(last_upload_completion_);
    const ResourceUploadCommitStatus commit_status = commit_queue_.Submit(commit_request);
    snapshot_.last_commit_status = commit_status;
    if (commit_status != ResourceUploadCommitStatus::Queued) {
        return RecordFailed(ResourceStreamingPipelineStatus::CommitSubmitFailed);
    }

    snapshot_.last_status = ResourceStreamingPipelineStatus::Queued;
    return ResourceStreamingPipelineStatus::Queued;
}

ResourceStreamingPipelineStatus ResourceStreamingPipeline::ProcessCommit() {
    if (!has_active_request_) {
        return RecordRejected(ResourceStreamingPipelineStatus::NotSubmitted);
    }

    const ResourceUploadCommitStatus commit_status = commit_queue_.ProcessNext();
    snapshot_.last_commit_status = commit_status;

    std::array<ResourceUploadCommitCompletion, 1U> completions{};
    std::uint32_t written_count = 0U;
    const ResourceUploadCommitStatus drain_status = commit_queue_.DrainCompletions(
        completions.data(),
        static_cast<std::uint32_t>(completions.size()),
        &written_count);
    if (drain_status != ResourceUploadCommitStatus::Success) {
        snapshot_.last_commit_status = drain_status;
        return RecordFailed(ResourceStreamingPipelineStatus::CommitCompletionMissing);
    }

    if (written_count != 1U) {
        return RecordFailed(ResourceStreamingPipelineStatus::CommitCompletionMissing);
    }

    last_commit_completion_ = completions[0U];
    if (commit_status != ResourceUploadCommitStatus::Success) {
        return RecordFailed(ResourceStreamingPipelineStatus::CommitProcessFailed);
    }

    if (last_upload_completion_.status != ResourceUploadStatus::Success) {
        return RecordCompleted(ResourceStreamingPipelineStatus::UploadProcessFailed);
    }

    return RecordCompleted(ResourceStreamingPipelineStatus::Success);
}

ResourceStreamingPipelineSnapshot ResourceStreamingPipeline::Snapshot() const {
    return snapshot_;
}

PackageResourceStagingCompletion ResourceStreamingPipeline::LastStagingCompletion() const {
    return last_staging_completion_;
}

ResourceUploadCompletion ResourceStreamingPipeline::LastUploadCompletion() const {
    return last_upload_completion_;
}

ResourceUploadCommitCompletion ResourceStreamingPipeline::LastCommitCompletion() const {
    return last_commit_completion_;
}

ResourceStreamingPipelineStatus ResourceStreamingPipeline::ValidateRequest(
    const ResourceStreamingPipelineRequest &request) const {
    if (request.resource_registry == nullptr) {
        return ResourceStreamingPipelineStatus::InvalidArgument;
    }

    if (request.file_queue == nullptr) {
        return ResourceStreamingPipelineStatus::InvalidArgument;
    }

    if (request.rhi_device == nullptr) {
        return ResourceStreamingPipelineStatus::InvalidArgument;
    }

    if (request.staging_request_id == 0U) {
        return ResourceStreamingPipelineStatus::InvalidArgument;
    }

    if (request.upload_id == 0U) {
        return ResourceStreamingPipelineStatus::InvalidArgument;
    }

    if (request.commit_id == 0U) {
        return ResourceStreamingPipelineStatus::InvalidArgument;
    }

    if (request.upload_byte_count > 0U && request.staged_bytes.data() == nullptr) {
        return ResourceStreamingPipelineStatus::InvalidArgument;
    }

    return ResourceStreamingPipelineStatus::Success;
}

ResourceStreamingPipelineStatus ResourceStreamingPipeline::RecordRejected(
    ResourceStreamingPipelineStatus status) {
    ++snapshot_.rejected_count;
    snapshot_.last_status = status;
    return status;
}

ResourceStreamingPipelineStatus ResourceStreamingPipeline::RecordFailed(
    ResourceStreamingPipelineStatus status) {
    ++snapshot_.failed_count;
    snapshot_.last_status = status;
    ClearActiveRequest();
    return status;
}

ResourceStreamingPipelineStatus ResourceStreamingPipeline::RecordCompleted(
    ResourceStreamingPipelineStatus status) {
    if (status == ResourceStreamingPipelineStatus::Success) {
        ++snapshot_.completed_count;
    }

    if (status != ResourceStreamingPipelineStatus::Success) {
        ++snapshot_.failed_count;
    }

    snapshot_.last_status = status;
    ClearActiveRequest();
    return status;
}

void ResourceStreamingPipeline::ClearActiveRequest() {
    request_ = ResourceStreamingPipelineRequest{};
    has_active_request_ = false;
    snapshot_.has_active_request = false;
}

PackageResourceStagingRequest ResourceStreamingPipeline::BuildStagingRequest(
    const ResourceStreamingPipelineRequest &request) const {
    PackageResourceStagingRequest staging_request;
    staging_request.resource_registry = request.resource_registry;
    staging_request.file_queue = request.file_queue;
    staging_request.package_record = request.package_record;
    staging_request.resource = request.resource;
    staging_request.expected_type = request.expected_type;
    staging_request.file_request = request.file_request;
    staging_request.request_id = request.staging_request_id;
    return staging_request;
}

ResourceUploadRequest ResourceStreamingPipeline::BuildUploadRequest(
    const PackageResourceStagingCompletion &completion) const {
    ResourceUploadRequest upload_request;
    upload_request.resource_registry = request_.resource_registry;
    upload_request.rhi_device = request_.rhi_device;
    upload_request.staging_completion = completion;
    upload_request.resource = request_.resource;
    upload_request.expected_type = request_.expected_type;
    upload_request.staged_bytes = request_.staged_bytes;
    upload_request.upload_byte_offset = request_.upload_byte_offset;
    upload_request.upload_byte_count = request_.upload_byte_count;
    upload_request.upload_kind = request_.upload_kind;
    upload_request.buffer_desc = request_.buffer_desc;
    upload_request.input_buffer_handle = request_.input_buffer_handle;
    upload_request.output_buffer_handle = request_.output_buffer_handle;
    upload_request.texture_desc = request_.texture_desc;
    upload_request.input_texture_handle = request_.input_texture_handle;
    upload_request.output_texture_handle = request_.output_texture_handle;
    upload_request.output_fence = request_.output_fence;
    upload_request.upload_id = request_.upload_id;
    return upload_request;
}

ResourceUploadCommitRequest ResourceStreamingPipeline::BuildCommitRequest(
    const ResourceUploadCompletion &completion) const {
    ResourceUploadCommitRequest commit_request;
    commit_request.resource_registry = request_.resource_registry;
    commit_request.upload_completion = completion;
    commit_request.commit_id = request_.commit_id;
    return commit_request;
}
}
