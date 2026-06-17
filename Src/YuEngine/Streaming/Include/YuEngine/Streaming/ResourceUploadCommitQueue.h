// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadCommitQueue.h

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
     * @comment Constructs a queue with default bounded storage.
     */
    ResourceUploadCommitQueue();
    /**
     * @comment Constructs a queue with bounded storage limits.
     * @param desc Input descriptor.
     */
    explicit ResourceUploadCommitQueue(ResourceUploadCommitQueueDesc desc);

    /**
     * @comment Validates and queues a Resource upload completion commit.
     * @param request Input request.
     * @return Explicit operation status.
     */
    ResourceUploadCommitStatus Submit(const ResourceUploadCommitRequest &request);
    /**
     * @comment Processes the oldest queued upload completion commit.
     * @return Explicit operation status.
     */
    ResourceUploadCommitStatus ProcessNext();
    /**
     * @comment Drains commit completion records into caller-owned storage.
     * @param output_completions Output completion storage.
     * @param output_capacity Output storage capacity.
     * @param written_count Output written count.
     * @return Explicit operation status.
     */
    ResourceUploadCommitStatus DrainCompletions(
        ResourceUploadCommitCompletion *output_completions,
        std::uint32_t output_capacity,
        std::uint32_t *written_count);
    /**
     * @comment Returns a queue snapshot value.
     * @return Snapshot value.
     */
    ResourceUploadCommitSnapshot Snapshot() const;

private:
    struct PendingRecord {
        bool is_active = false;
        std::uint64_t sequence_id = 0U;
        ResourceUploadCommitRequest request;
    };

    ResourceUploadCommitStatus RecordRejected(ResourceUploadCommitStatus status);
    ResourceUploadCommitStatus RecordCompletionOverflow();
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
