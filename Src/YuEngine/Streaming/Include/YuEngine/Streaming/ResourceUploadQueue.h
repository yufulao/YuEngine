// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadQueue.h

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
     * @comment Constructs a queue with default bounded storage.
     */
    ResourceUploadQueue();
    /**
     * @comment Constructs a queue with bounded storage limits.
     * @param desc Input descriptor.
     */
    explicit ResourceUploadQueue(ResourceUploadQueueDesc desc);

    /**
     * @comment Validates and queues a Resource upload request.
     * @param request Input request.
     * @return Explicit operation status.
     */
    ResourceUploadStatus Submit(const ResourceUploadRequest &request);
    /**
     * @comment Processes the oldest queued upload request.
     * @return Explicit operation status.
     */
    ResourceUploadStatus ProcessNext();
    /**
     * @comment Drains upload completion records into caller-owned storage.
     * @param output_completions Output completion storage.
     * @param output_capacity Output storage capacity.
     * @param written_count Output written count.
     * @return Explicit operation status.
     */
    ResourceUploadStatus DrainCompletions(
        ResourceUploadCompletion *output_completions,
        std::uint32_t output_capacity,
        std::uint32_t *written_count);
    /**
     * @comment Returns a queue snapshot value.
     * @return Snapshot value.
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
    ResourceUploadStatus RecordCompletionOverflow();
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
