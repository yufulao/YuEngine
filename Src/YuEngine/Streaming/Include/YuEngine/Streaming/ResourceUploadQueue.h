// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadQueue.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Streaming/ResourceUploadCompletion.h"
#include "YuEngine/Streaming/ResourceUploadConstants.h"
#include "YuEngine/Streaming/ResourceUploadQueueDesc.h"
#include "YuEngine/Streaming/ResourceUploadRequest.h"
#include "YuEngine/Streaming/ResourceUploadSnapshot.h"
#include "YuEngine/Streaming/ResourceUploadStatus.h"

namespace yuengine::streaming {
class ResourceUploadQueue final {
public:
    /**
     * @comment 使用默认固定存储构造队列。
     */
    ResourceUploadQueue();
    /**
     * @comment 使用固定存储上限构造队列。
     * @param desc 输入描述。
     */
    explicit ResourceUploadQueue(ResourceUploadQueueDesc desc);

    /**
     * @comment 验证并入队 Resource upload 请求。
     * @param request 输入请求。
     * @return 显式操作状态。
     */
    ResourceUploadStatus Submit(const ResourceUploadRequest &request);
    /**
     * @comment 处理最早入队的 upload 请求。
     * @return 显式操作状态。
     */
    ResourceUploadStatus ProcessNext();
    /**
     * @comment 提取 upload completion 记录并写入调用方持有的存储。
     * @param output_completions 输出 completion 存储。
     * @param output_capacity 输出存储容量。
     * @param written_count 输出写入数量。
     * @return 显式操作状态。
     */
    ResourceUploadStatus DrainCompletions(
        ResourceUploadCompletion *output_completions,
        std::uint32_t output_capacity,
        std::uint32_t *written_count);
    /**
     * @comment 返回队列快照值。
     * @return 快照值。
     */
    ResourceUploadSnapshot Snapshot() const;

private:
    struct PendingRecord {
        bool is_active = false;
        ResourceUploadRequest request;
    };

    ResourceUploadStatus RecordRejected(ResourceUploadStatus status);
    ResourceUploadStatus RecordRejected(
        ResourceUploadStatus status,
        resource::ResourceStatus resource_status);
    ResourceUploadStatus RecordCompletionOverflow(const ResourceUploadRequest &request);
    ResourceUploadStatus ValidateRequest(const ResourceUploadRequest &request) const;
    bool HasUploadId(std::uint64_t upload_id) const;
    PendingRecord *FindFreePendingRecord();
    PendingRecord *FindOldestPendingRecord();
    void StorePendingRecord(const ResourceUploadRequest &request);
    bool AppendCompletion(const ResourceUploadCompletion &completion);
    void RemovePendingRecord(PendingRecord &pending_record);
    ResourceUploadCompletion BuildBaseCompletion(const ResourceUploadRequest &request) const;
    ResourceUploadStatus ProcessPendingRecord(PendingRecord &pending_record);
    ResourceUploadStatus ProcessCreateBuffer(
        PendingRecord &pending_record,
        ResourceUploadRequest &request,
        ResourceUploadCompletion &completion);
    ResourceUploadStatus ProcessUpdateBuffer(
        PendingRecord &pending_record,
        ResourceUploadRequest &request,
        ResourceUploadCompletion &completion);
    ResourceUploadStatus ProcessCreateTexture(
        PendingRecord &pending_record,
        ResourceUploadRequest &request,
        ResourceUploadCompletion &completion);
    ResourceUploadStatus ProcessUpdateTexture(
        PendingRecord &pending_record,
        ResourceUploadRequest &request,
        ResourceUploadCompletion &completion);
    void FinishCompletedRecord(PendingRecord &pending_record, const ResourceUploadCompletion &completion);
    void FinishFailedRecord(PendingRecord &pending_record, const ResourceUploadCompletion &completion);
    void UpdateMaxPendingCount();
    void UpdateMaxCompletionCount();

    std::array<PendingRecord, MAX_RESOURCE_UPLOAD_REQUEST_COUNT> pending_records_;
    std::array<ResourceUploadCompletion, MAX_RESOURCE_UPLOAD_COMPLETION_COUNT> completions_;
    ResourceUploadSnapshot snapshot_;
};
}
