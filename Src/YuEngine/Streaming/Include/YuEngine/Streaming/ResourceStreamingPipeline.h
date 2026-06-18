// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceStreamingPipeline.h

#pragma once

#include "YuEngine/File/AsyncFileReadResult.h"
#include "YuEngine/Streaming/PackageResourceStagingCompletion.h"
#include "YuEngine/Streaming/PackageResourceStagingQueue.h"
#include "YuEngine/Streaming/ResourceStreamingPipelineRequest.h"
#include "YuEngine/Streaming/ResourceStreamingPipelineSnapshot.h"
#include "YuEngine/Streaming/ResourceStreamingPipelineStatus.h"
#include "YuEngine/Streaming/ResourceUploadCommitCompletion.h"
#include "YuEngine/Streaming/ResourceUploadCommitQueue.h"
#include "YuEngine/Streaming/ResourceUploadCompletion.h"
#include "YuEngine/Streaming/ResourceUploadQueue.h"

namespace yuengine::streaming {
class ResourceStreamingPipeline final {
public:
    /**
     * @comment Constructs a Resource streaming pipeline with bounded internal queues.
     */
    ResourceStreamingPipeline();

    /**
     * @comment Submits one package-backed resource upload pipeline request.
     * @param request Input pipeline request.
     * @return Explicit pipeline status.
     */
    ResourceStreamingPipelineStatus Submit(const ResourceStreamingPipelineRequest &request);
    /**
     * @comment Consumes a caller-drained async file completion and queues the upload step.
     * @param file_result Input async file completion.
     * @return Explicit pipeline status.
     */
    ResourceStreamingPipelineStatus CompleteFileRead(const file::AsyncFileReadResult &file_result);
    /**
     * @comment Processes the queued RHI upload step and queues the Resource commit step.
     * @return Explicit pipeline status.
     */
    ResourceStreamingPipelineStatus ProcessUpload();
    /**
     * @comment Processes the queued Resource commit step.
     * @return Explicit pipeline status.
     */
    ResourceStreamingPipelineStatus ProcessCommit();
    /**
     * @comment Returns pipeline counters and last statuses.
     * @return Snapshot value.
     */
    ResourceStreamingPipelineSnapshot Snapshot() const;
    /**
     * @comment Returns the last staging completion.
     * @return Completion value.
     */
    PackageResourceStagingCompletion LastStagingCompletion() const;
    /**
     * @comment Returns the last upload completion.
     * @return Completion value.
     */
    ResourceUploadCompletion LastUploadCompletion() const;
    /**
     * @comment Returns the last commit completion.
     * @return Completion value.
     */
    ResourceUploadCommitCompletion LastCommitCompletion() const;

private:
    ResourceStreamingPipelineStatus ValidateRequest(const ResourceStreamingPipelineRequest &request) const;
    ResourceStreamingPipelineStatus RecordRejected(ResourceStreamingPipelineStatus status);
    ResourceStreamingPipelineStatus RecordFailed(ResourceStreamingPipelineStatus status);
    ResourceStreamingPipelineStatus RecordCompleted(ResourceStreamingPipelineStatus status);
    void ClearActiveRequest();
    PackageResourceStagingRequest BuildStagingRequest(const ResourceStreamingPipelineRequest &request) const;
    ResourceUploadRequest BuildUploadRequest(const PackageResourceStagingCompletion &completion) const;
    ResourceUploadCommitRequest BuildCommitRequest(const ResourceUploadCompletion &completion) const;

    PackageResourceStagingQueue staging_queue_;
    ResourceUploadQueue upload_queue_;
    ResourceUploadCommitQueue commit_queue_;
    ResourceStreamingPipelineRequest request_;
    PackageResourceStagingCompletion last_staging_completion_;
    ResourceUploadCompletion last_upload_completion_;
    ResourceUploadCommitCompletion last_commit_completion_;
    ResourceStreamingPipelineSnapshot snapshot_;
    bool has_active_request_;
};
}
