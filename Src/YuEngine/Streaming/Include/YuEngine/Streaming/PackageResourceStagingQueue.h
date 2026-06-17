// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/PackageResourceStagingQueue.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/File/AsyncFileReadResult.h"
#include "YuEngine/Streaming/PackageResourceStagingCompletion.h"
#include "YuEngine/Streaming/PackageResourceStagingConstants.h"
#include "YuEngine/Streaming/PackageResourceStagingQueueDesc.h"
#include "YuEngine/Streaming/PackageResourceStagingRequest.h"
#include "YuEngine/Streaming/PackageResourceStagingSnapshot.h"
#include "YuEngine/Streaming/PackageResourceStagingStatus.h"

namespace yuengine::streaming {
class PackageResourceStagingQueue final {
public:
    /**
     * @comment Constructs a queue with default bounded storage.
     */
    PackageResourceStagingQueue();
    /**
     * @comment Constructs a queue with bounded storage limits.
     * @param desc Input descriptor.
     */
    explicit PackageResourceStagingQueue(PackageResourceStagingQueueDesc desc);

    /**
     * @comment Validates and submits a package-resource staging request.
     * @param request Input request.
     * @return Explicit operation status.
     */
    PackageResourceStagingStatus Submit(const PackageResourceStagingRequest &request);
    /**
     * @comment Accepts a caller-drained async file read completion.
     * @param file_result Input file completion value.
     * @return Explicit operation status.
     */
    PackageResourceStagingStatus CompleteFileRead(const file::AsyncFileReadResult &file_result);
    /**
     * @comment Drains staging completion records into caller-owned storage.
     * @param output_completions Output completion storage.
     * @param output_capacity Output storage capacity.
     * @param written_count Output written count.
     * @return Explicit operation status.
     */
    PackageResourceStagingStatus DrainCompletions(
        PackageResourceStagingCompletion *output_completions,
        std::uint32_t output_capacity,
        std::uint32_t *written_count);
    /**
     * @comment Returns a queue snapshot value.
     * @return Snapshot value.
     */
    PackageResourceStagingSnapshot Snapshot() const;

private:
    struct PendingRecord {
        bool is_active = false;
        package::PackageLoadPlanRecord package_record;
        resource::ResourceHandle resource;
        resource::ResourceTypeId expected_type;
        std::uint64_t request_id = 0U;
    };

    PackageResourceStagingStatus RecordRejected(PackageResourceStagingStatus status);
    PackageResourceStagingStatus RecordRejected(
        PackageResourceStagingStatus status,
        resource::ResourceStatus resource_status);
    PackageResourceStagingStatus RecordFileSubmitFailure(file::AsyncFileReadStatus async_file_status);
    PackageResourceStagingStatus RecordMissingCompletion();
    PackageResourceStagingStatus RecordCompletionOverflow();
    PackageResourceStagingStatus ValidateRequest(const PackageResourceStagingRequest &request) const;
    bool HasRequestId(std::uint64_t request_id) const;
    PendingRecord *FindFreePendingRecord();
    PendingRecord *FindPendingRecord(std::uint64_t request_id);
    void StorePendingRecord(const PackageResourceStagingRequest &request);
    bool AppendCompletion(const PackageResourceStagingCompletion &completion);
    void RemovePendingRecord(PendingRecord &pending_record);
    void UpdateMaxPendingCount();
    void UpdateMaxCompletionCount();

    std::array<PendingRecord, MAX_PACKAGE_RESOURCE_STAGING_REQUEST_COUNT> pending_records_;
    std::array<PackageResourceStagingCompletion, MAX_PACKAGE_RESOURCE_STAGING_COMPLETION_COUNT> completions_;
    PackageResourceStagingSnapshot snapshot_;
};
}
