// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceStreamingPipeline.h

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
     * @comment 构造带固定容量内部队列的 Resource streaming pipeline。
     */
    ResourceStreamingPipeline();

    /**
     * @comment 提交 package-backed resource upload 流水线请求。
     * @param request 输入流水线请求。
     * @return 显式流水线状态。
     */
    ResourceStreamingPipelineStatus Submit(const ResourceStreamingPipelineRequest &request);
    /**
     * @comment 消费调用方已排空的异步文件完成记录，并入队 upload 步骤。
     * @param file_result 输入 async file completion。
     * @return 显式流水线状态。
     */
    ResourceStreamingPipelineStatus CompleteFileRead(const file::AsyncFileReadResult &file_result);
    /**
     * @comment 处理入队的 RHI upload 步骤，并入队 Resource commit 步骤。
     * @return 显式流水线状态。
     */
    ResourceStreamingPipelineStatus ProcessUpload();
    /**
     * @comment 处理入队的 Resource commit 步骤。
     * @return 显式流水线状态。
     */
    ResourceStreamingPipelineStatus ProcessCommit();
    /**
     * @comment 返回 pipeline 计数器和最近状态。
     * @return 快照值。
     */
    ResourceStreamingPipelineSnapshot Snapshot() const;
    /**
     * @comment 返回最近的 staging completion。
     * @return Completion 值。
     */
    PackageResourceStagingCompletion LastStagingCompletion() const;
    /**
     * @comment 返回最近的 upload completion。
     * @return Completion 值。
     */
    ResourceUploadCompletion LastUploadCompletion() const;
    /**
     * @comment 返回最近的 commit completion。
     * @return Completion 值。
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
