// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadCommitQueue.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Streaming/ResourceUploadCommitCompletion.h"
#include "YuEngine/Streaming/ResourceUploadCommitConstants.h"
#include "YuEngine/Streaming/ResourceUploadCommitQueueDesc.h"
#include "YuEngine/Streaming/ResourceUploadCommitRequest.h"
#include "YuEngine/Streaming/ResourceUploadCommitSnapshot.h"
#include "YuEngine/Streaming/ResourceUploadCommitStatus.h"

namespace yuengine::streaming {
class ResourceUploadCommitQueue final {
public:
    /**
     * @comment 使用默认固定存储构造队列。
     */
    ResourceUploadCommitQueue();
    /**
     * @comment 使用固定存储上限构造队列。
     * @param desc 输入描述。
     */
    explicit ResourceUploadCommitQueue(ResourceUploadCommitQueueDesc desc);

    /**
     * @comment 验证并入队 Resource upload completion commit。
     * @param request 输入请求。
     * @return 显式操作状态。
     */
    ResourceUploadCommitStatus Submit(const ResourceUploadCommitRequest &request);
    /**
     * @comment 处理最早入队的 upload completion commit。
     * @return 显式操作状态。
     */
    ResourceUploadCommitStatus ProcessNext();
    /**
     * @comment 提取 commit completion 记录并写入调用方持有的存储。
     * @param output_completions 输出 completion 存储。
     * @param output_capacity 输出存储容量。
     * @param written_count 输出写入数量。
     * @return 显式操作状态。
     */
    ResourceUploadCommitStatus DrainCompletions(
        ResourceUploadCommitCompletion *output_completions,
        std::uint32_t output_capacity,
        std::uint32_t *written_count);
    /**
     * @comment 返回队列快照值。
     * @return 快照值。
     */
    ResourceUploadCommitSnapshot Snapshot() const;

private:
    struct PendingRecord {
        bool is_active = false;
        std::uint64_t sequence_id = 0U;
        ResourceUploadCommitRequest request;
    };

    ResourceUploadCommitStatus RecordRejected(ResourceUploadCommitStatus status);
    ResourceUploadCommitStatus RecordCompletionOverflow(const ResourceUploadCommitRequest &request);
    ResourceUploadCommitStatus ValidateRequest(const ResourceUploadCommitRequest &request) const;
    bool HasCommitId(std::uint64_t commit_id) const;
    PendingRecord *FindFreePendingRecord();
    PendingRecord *FindOldestPendingRecord();
    void StorePendingRecord(const ResourceUploadCommitRequest &request);
    bool AppendCompletion(const ResourceUploadCommitCompletion &completion);
    void RemovePendingRecord(PendingRecord &pending_record);
    ResourceUploadCommitCompletion BuildBaseCompletion(const ResourceUploadCommitRequest &request) const;
    ResourceUploadCommitStatus ProcessPendingRecord(PendingRecord &pending_record);
    void FinishCompletedRecord(PendingRecord &pending_record, const ResourceUploadCommitCompletion &completion);
    void FinishFailedRecord(PendingRecord &pending_record, const ResourceUploadCommitCompletion &completion);
    void UpdateMaxPendingCount();
    void UpdateMaxCompletionCount();

    std::array<PendingRecord, MAX_RESOURCE_UPLOAD_COMMIT_REQUEST_COUNT> pending_records_;
    std::array<ResourceUploadCommitCompletion, MAX_RESOURCE_UPLOAD_COMMIT_COMPLETION_COUNT> completions_;
    ResourceUploadCommitSnapshot snapshot_;
    std::uint64_t next_pending_sequence_;
};
}
