// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Src/PackageResourceStagingQueue.cpp

#include "YuEngine/Streaming/PackageResourceStagingQueue.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "YuEngine/File/AsyncFileReadQueue.h"
#include "YuEngine/Resource/ResourceRegistry.h"

namespace yuengine::streaming {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}

bool ByteRangeOverflows(std::uint64_t byte_offset, std::uint64_t byte_size) {
    return byte_offset > std::numeric_limits<std::uint64_t>::max() - byte_size;
}

bool ByteCountExceedsCapacity(std::uint64_t byte_count, std::size_t capacity) {
    return byte_count > static_cast<std::uint64_t>(capacity);
}

bool ByteCountExceedsUInt32(std::uint64_t byte_count) {
    return byte_count > static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max());
}

file::AsyncFileReadRequest BuildStagingFileRequest(const PackageResourceStagingRequest &request) {
    file::AsyncFileReadRequest file_request = request.file_request;
    file_request.read_request.use_range = true;
    file_request.read_request.range_byte_offset = request.package_record.archive_byte_offset;
    file_request.read_request.range_byte_size = request.package_record.archive_byte_size;
    file_request.request_index = request.request_id;
    return file_request;
}

bool HasEarlierBatchRequestId(
    const PackageResourceStagingRequest *requests,
    std::uint32_t request_index) {
    for (std::uint32_t index = 0U; index < request_index; ++index) {
        if (requests[index].request_id == requests[request_index].request_id) {
            return true;
        }
    }

    return false;
}

bool HasEarlierBatchFileQueue(
    const PackageResourceStagingRequest *requests,
    std::uint32_t request_index) {
    for (std::uint32_t index = 0U; index < request_index; ++index) {
        if (requests[index].file_queue == requests[request_index].file_queue) {
            return true;
        }
    }

    return false;
}

std::size_t CountBatchRequestsForFileQueue(
    const PackageResourceStagingRequest *requests,
    std::uint32_t request_count,
    file::AsyncFileReadQueue *file_queue) {
    std::size_t result = 0U;
    for (std::uint32_t index = 0U; index < request_count; ++index) {
        if (requests[index].file_queue == file_queue) {
            ++result;
        }
    }

    return result;
}

file::AsyncFileReadStatus SelectFileQueueReadinessStatus(
    const file::AsyncFileReadQueueSnapshot &snapshot) {
    if (!snapshot.is_initialized) {
        return file::AsyncFileReadStatus::NotInitialized;
    }

    if (snapshot.is_shutdown) {
        return file::AsyncFileReadStatus::ShutdownRequested;
    }

    if (!snapshot.is_started) {
        return file::AsyncFileReadStatus::NotStarted;
    }

    return file::AsyncFileReadStatus::Success;
}

bool HasFileQueueCapacity(
    const file::AsyncFileReadQueueSnapshot &snapshot,
    std::size_t request_count) {
    if (snapshot.pending_count > snapshot.work_capacity) {
        return false;
    }

    const std::size_t available_count = snapshot.work_capacity - snapshot.pending_count;
    return request_count <= available_count;
}
}

PackageResourceStagingQueue::PackageResourceStagingQueue()
    : PackageResourceStagingQueue(PackageResourceStagingQueueDesc{}) {
}

PackageResourceStagingQueue::PackageResourceStagingQueue(PackageResourceStagingQueueDesc desc)
    : pending_records_{},
      completions_{},
      snapshot_{
          ClampCapacity(desc.request_capacity, MAX_PACKAGE_RESOURCE_STAGING_REQUEST_COUNT),
          ClampCapacity(desc.completion_capacity, MAX_PACKAGE_RESOURCE_STAGING_COMPLETION_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          PackageResourceStagingStatus::Success,
          resource::ResourceStatus::Success,
          file::AsyncFileReadStatus::Success,
          file::FileStatus::Success,
          memory::MemoryAccountingStatus::ExplicitlyTrackedOnly} {
}

PackageResourceStagingStatus PackageResourceStagingQueue::Submit(
    const PackageResourceStagingRequest &request) {
    const PackageResourceStagingStatus request_status = ValidateRequest(request);
    if (request_status != PackageResourceStagingStatus::Success) {
        return RecordRejected(request_status);
    }

    const resource::ResourceStatus resource_status =
        request.resource_registry->ValidateAcquire(request.resource, request.expected_type, 0U);
    if (resource_status != resource::ResourceStatus::Success) {
        return RecordRejected(PackageResourceStagingStatus::ResourceValidationFailed, resource_status);
    }

    if (HasRequestId(request.request_id)) {
        ++snapshot_.duplicate_request_count;
        return RecordRejected(PackageResourceStagingStatus::DuplicateRequestId);
    }

    if (snapshot_.pending_count >= snapshot_.request_capacity) {
        return RecordRejected(PackageResourceStagingStatus::QueueFull);
    }

    const file::AsyncFileReadRequest file_request = BuildStagingFileRequest(request);
    const file::AsyncFileReadStatus async_file_status = request.file_queue->Submit(file_request);
    if (async_file_status != file::AsyncFileReadStatus::Queued) {
        return RecordFileSubmitFailure(async_file_status);
    }

    StorePendingRecord(request);
    ++snapshot_.submitted_count;
    snapshot_.last_status = PackageResourceStagingStatus::Queued;
    snapshot_.last_resource_status = resource::ResourceStatus::Success;
    snapshot_.last_async_file_status = async_file_status;
    snapshot_.last_file_status = file::FileStatus::Success;
    return PackageResourceStagingStatus::Queued;
}

PackageResourceStagingBatchSubmitResult PackageResourceStagingQueue::SubmitBatch(
    const PackageResourceStagingRequest *requests,
    std::uint32_t request_count,
    PackageResourceStagingSubmitResult *output_results,
    std::uint32_t output_capacity) {
    PackageResourceStagingBatchSubmitResult result;
    result.request_count = request_count;
    result.required_result_count = request_count;

    if (request_count == 0U) {
        result.status = PackageResourceStagingStatus::InvalidArgument;
        return result;
    }

    if (requests == nullptr) {
        result.status = PackageResourceStagingStatus::InvalidArgument;
        return result;
    }

    if (output_results == nullptr) {
        result.status = PackageResourceStagingStatus::InvalidArgument;
        return result;
    }

    if (output_capacity < request_count) {
        result.status = PackageResourceStagingStatus::OutputTooSmall;
        return result;
    }

    for (std::uint32_t index = 0U; index < request_count; ++index) {
        const PackageResourceStagingRequest &request = requests[index];
        const PackageResourceStagingStatus request_status = ValidateRequest(request);
        if (request_status != PackageResourceStagingStatus::Success) {
            result.status = request_status;
            result.failed_index = index;
            result.failed_request_id = request.request_id;
            return result;
        }

        const resource::ResourceStatus resource_status =
            request.resource_registry->ValidateAcquire(request.resource, request.expected_type, 0U);
        if (resource_status != resource::ResourceStatus::Success) {
            result.status = PackageResourceStagingStatus::ResourceValidationFailed;
            result.resource_status = resource_status;
            result.failed_index = index;
            result.failed_request_id = request.request_id;
            return result;
        }

        if (HasRequestId(request.request_id)) {
            result.status = PackageResourceStagingStatus::DuplicateRequestId;
            result.failed_index = index;
            result.failed_request_id = request.request_id;
            return result;
        }

        if (HasEarlierBatchRequestId(requests, index)) {
            result.status = PackageResourceStagingStatus::DuplicateRequestId;
            result.failed_index = index;
            result.failed_request_id = request.request_id;
            return result;
        }
    }

    if (snapshot_.pending_count > snapshot_.request_capacity) {
        result.status = PackageResourceStagingStatus::QueueFull;
        return result;
    }

    const std::uint32_t available_request_count = snapshot_.request_capacity - snapshot_.pending_count;
    if (request_count > available_request_count) {
        result.status = PackageResourceStagingStatus::QueueFull;
        return result;
    }

    for (std::uint32_t index = 0U; index < request_count; ++index) {
        if (HasEarlierBatchFileQueue(requests, index)) {
            continue;
        }

        const PackageResourceStagingRequest &request = requests[index];
        const file::AsyncFileReadQueueSnapshot file_snapshot = request.file_queue->Snapshot();
        const file::AsyncFileReadStatus readiness_status = SelectFileQueueReadinessStatus(file_snapshot);
        if (readiness_status != file::AsyncFileReadStatus::Success) {
            result.status = PackageResourceStagingStatus::FileSubmitFailed;
            result.async_file_status = readiness_status;
            result.failed_index = index;
            result.failed_request_id = request.request_id;
            return result;
        }

        const std::size_t file_request_count =
            CountBatchRequestsForFileQueue(requests, request_count, request.file_queue);
        if (!HasFileQueueCapacity(file_snapshot, file_request_count)) {
            result.status = PackageResourceStagingStatus::FileSubmitFailed;
            result.async_file_status = file::AsyncFileReadStatus::QueueFull;
            result.failed_index = index;
            result.failed_request_id = request.request_id;
            return result;
        }
    }

    std::array<PackageResourceStagingSubmitResult, MAX_PACKAGE_RESOURCE_STAGING_REQUEST_COUNT> submit_results{};
    for (std::uint32_t index = 0U; index < request_count; ++index) {
        const PackageResourceStagingRequest &request = requests[index];
        const file::AsyncFileReadRequest file_request = BuildStagingFileRequest(request);
        const file::AsyncFileReadStatus async_file_status = request.file_queue->Submit(file_request);
        if (async_file_status != file::AsyncFileReadStatus::Queued) {
            result.status = PackageResourceStagingStatus::FileSubmitFailed;
            result.async_file_status = async_file_status;
            result.failed_index = index;
            result.failed_request_id = request.request_id;
            return result;
        }

        StorePendingRecord(request);
        ++snapshot_.submitted_count;
        submit_results[index].status = PackageResourceStagingStatus::Queued;
        submit_results[index].async_file_status = async_file_status;
        submit_results[index].request_id = request.request_id;
        ++result.submitted_count;
    }

    for (std::uint32_t index = 0U; index < request_count; ++index) {
        output_results[index] = submit_results[index];
    }

    snapshot_.last_status = PackageResourceStagingStatus::Queued;
    snapshot_.last_resource_status = resource::ResourceStatus::Success;
    snapshot_.last_async_file_status = file::AsyncFileReadStatus::Queued;
    snapshot_.last_file_status = file::FileStatus::Success;
    result.status = PackageResourceStagingStatus::Queued;
    result.async_file_status = file::AsyncFileReadStatus::Queued;
    return result;
}

PackageResourceStagingStatus PackageResourceStagingQueue::CompleteFileRead(
    const file::AsyncFileReadResult &file_result) {
    PendingRecord *pending_record = FindPendingRecord(file_result.request_index);
    if (pending_record == nullptr) {
        return RecordMissingCompletion();
    }

    if (snapshot_.completion_count >= snapshot_.completion_capacity) {
        return RecordCompletionOverflow();
    }

    PackageResourceStagingCompletion completion;
    completion.package_record = pending_record->package_record;
    completion.resource = pending_record->resource;
    completion.expected_type = pending_record->expected_type;
    completion.request_id = pending_record->request_id;
    completion.file_byte_count = file_result.byte_count;
    completion.async_file_status = file_result.status;
    completion.file_status = file_result.file_status;

    if (file_result.status != file::AsyncFileReadStatus::Success) {
        completion.status = PackageResourceStagingStatus::FileReadFailed;
        AppendCompletion(completion);
        RemovePendingRecord(*pending_record);
        ++snapshot_.failed_count;
        snapshot_.last_status = completion.status;
        snapshot_.last_async_file_status = file_result.status;
        snapshot_.last_file_status = file_result.file_status;
        return completion.status;
    }

    const std::uint64_t archive_byte_size = pending_record->package_record.archive_byte_size;
    if (static_cast<std::uint64_t>(file_result.byte_count) != archive_byte_size) {
        completion.status = PackageResourceStagingStatus::FileByteCountMismatch;
        AppendCompletion(completion);
        RemovePendingRecord(*pending_record);
        ++snapshot_.failed_count;
        snapshot_.last_status = completion.status;
        snapshot_.last_async_file_status = file_result.status;
        snapshot_.last_file_status = file_result.file_status;
        return completion.status;
    }

    completion.status = PackageResourceStagingStatus::Success;
    completion.staged_byte_offset = 0U;
    completion.staged_byte_count = static_cast<std::uint32_t>(archive_byte_size);
    AppendCompletion(completion);
    RemovePendingRecord(*pending_record);
    ++snapshot_.completed_count;
    snapshot_.last_status = completion.status;
    snapshot_.last_async_file_status = file_result.status;
    snapshot_.last_file_status = file_result.file_status;
    return completion.status;
}

PackageResourceStagingStatus PackageResourceStagingQueue::DrainCompletions(
    PackageResourceStagingCompletion *output_completions,
    std::uint32_t output_capacity,
    std::uint32_t *written_count) {
    if (written_count == nullptr) {
        return PackageResourceStagingStatus::InvalidArgument;
    }

    if (snapshot_.completion_count == 0U) {
        snapshot_.required_completion_count = 0U;
        *written_count = 0U;
        return PackageResourceStagingStatus::Success;
    }

    if (output_completions == nullptr) {
        return PackageResourceStagingStatus::InvalidArgument;
    }

    if (output_capacity < snapshot_.completion_count) {
        snapshot_.required_completion_count = snapshot_.completion_count;
        snapshot_.last_status = PackageResourceStagingStatus::CompletionQueueFull;
        return PackageResourceStagingStatus::CompletionQueueFull;
    }

    *written_count = 0U;

    PackageResourceStagingStatus last_status = PackageResourceStagingStatus::Success;
    file::AsyncFileReadStatus last_async_file_status = file::AsyncFileReadStatus::Success;
    file::FileStatus last_file_status = file::FileStatus::Success;
    for (std::uint32_t index = 0U; index < snapshot_.completion_count; ++index) {
        const PackageResourceStagingCompletion completion = completions_[index];
        if (completion.status != PackageResourceStagingStatus::Success) {
            last_status = completion.status;
            last_async_file_status = completion.async_file_status;
            last_file_status = completion.file_status;
        }

        output_completions[index] = completion;
        ++(*written_count);
        completions_[index] = PackageResourceStagingCompletion{};
    }

    snapshot_.completion_count = 0U;
    snapshot_.required_completion_count = 0U;
    snapshot_.last_status = last_status;
    snapshot_.last_async_file_status = last_async_file_status;
    snapshot_.last_file_status = last_file_status;
    return PackageResourceStagingStatus::Success;
}

PackageResourceStagingStatus PackageResourceStagingQueue::GetPendingCountSnapshot(
    std::uint32_t *pending_count) const {
    if (pending_count == nullptr) {
        return PackageResourceStagingStatus::InvalidArgument;
    }

    *pending_count = snapshot_.pending_count;
    return PackageResourceStagingStatus::Success;
}

PackageResourceStagingPendingRequestEnumerationResult PackageResourceStagingQueue::EnumeratePendingRequests(
    PackageResourceStagingPendingRequestSnapshot *output_requests,
    std::uint32_t output_capacity,
    std::uint32_t *written_count) const {
    PackageResourceStagingPendingRequestEnumerationResult result;
    std::uint32_t pending_count = 0U;
    for (const PendingRecord &pending_record : pending_records_) {
        if (!pending_record.is_active) {
            continue;
        }

        ++pending_count;
    }

    result.required_request_count = pending_count;

    if (written_count == nullptr) {
        result.status = PackageResourceStagingStatus::InvalidArgument;
        return result;
    }

    if (output_requests == nullptr) {
        result.status = PackageResourceStagingStatus::InvalidArgument;
        return result;
    }

    if (output_capacity < pending_count) {
        result.status = PackageResourceStagingStatus::OutputTooSmall;
        return result;
    }

    std::uint32_t pending_index = 0U;
    for (const PendingRecord &pending_record : pending_records_) {
        if (!pending_record.is_active) {
            continue;
        }

        output_requests[pending_index].package_record = pending_record.package_record;
        output_requests[pending_index].resource = pending_record.resource;
        output_requests[pending_index].expected_type = pending_record.expected_type;
        output_requests[pending_index].request_id = pending_record.request_id;
        ++pending_index;
    }

    *written_count = pending_index;
    result.written_count = pending_index;
    result.required_request_count = pending_index;
    return result;
}

PackageResourceStagingSnapshot PackageResourceStagingQueue::Snapshot() const {
    return snapshot_;
}

PackageResourceStagingStatus PackageResourceStagingQueue::RecordRejected(
    PackageResourceStagingStatus status) {
    ++snapshot_.rejected_count;
    snapshot_.last_status = status;
    return status;
}

PackageResourceStagingStatus PackageResourceStagingQueue::RecordRejected(
    PackageResourceStagingStatus status,
    resource::ResourceStatus resource_status) {
    ++snapshot_.rejected_count;
    snapshot_.last_status = status;
    snapshot_.last_resource_status = resource_status;
    return status;
}

PackageResourceStagingStatus PackageResourceStagingQueue::RecordFileSubmitFailure(
    file::AsyncFileReadStatus async_file_status) {
    ++snapshot_.rejected_count;
    ++snapshot_.file_submit_failed_count;
    snapshot_.last_status = PackageResourceStagingStatus::FileSubmitFailed;
    snapshot_.last_async_file_status = async_file_status;
    return PackageResourceStagingStatus::FileSubmitFailed;
}

PackageResourceStagingStatus PackageResourceStagingQueue::RecordMissingCompletion() {
    ++snapshot_.missing_completion_count;
    snapshot_.last_status = PackageResourceStagingStatus::MissingFileCompletion;
    return PackageResourceStagingStatus::MissingFileCompletion;
}

PackageResourceStagingStatus PackageResourceStagingQueue::RecordCompletionOverflow() {
    snapshot_.last_status = PackageResourceStagingStatus::CompletionQueueFull;
    return PackageResourceStagingStatus::CompletionQueueFull;
}

PackageResourceStagingStatus PackageResourceStagingQueue::ValidateRequest(
    const PackageResourceStagingRequest &request) const {
    if (request.resource_registry == nullptr) {
        return PackageResourceStagingStatus::InvalidArgument;
    }

    if (request.file_queue == nullptr) {
        return PackageResourceStagingStatus::InvalidArgument;
    }

    if (request.file_request.mount_table == nullptr) {
        return PackageResourceStagingStatus::InvalidArgument;
    }

    if (request.package_record.archive_byte_size > 0ULL && request.file_request.output_bytes == nullptr) {
        return PackageResourceStagingStatus::InvalidArgument;
    }

    if (ByteCountExceedsCapacity(request.package_record.archive_byte_size, request.file_request.output_capacity)) {
        return PackageResourceStagingStatus::OutputTooSmall;
    }

    if (ByteCountExceedsUInt32(request.package_record.archive_byte_size)) {
        return PackageResourceStagingStatus::OutputTooSmall;
    }

    if (!request.package_record.package.IsValid()) {
        return PackageResourceStagingStatus::InvalidPackageRecord;
    }

    if (!request.package_record.entry.IsValid()) {
        return PackageResourceStagingStatus::InvalidPackageRecord;
    }

    if (!request.package_record.type.IsValid()) {
        return PackageResourceStagingStatus::InvalidPackageRecord;
    }

    if (!request.package_record.logical_key.IsWithinBounds()) {
        return PackageResourceStagingStatus::InvalidPackageRecord;
    }

    if (!request.package_record.logical_key.IsValid()) {
        return PackageResourceStagingStatus::InvalidPackageRecord;
    }

    if (!request.package_record.source_key.IsWithinBounds()) {
        return PackageResourceStagingStatus::InvalidPackageRecord;
    }

    if (!request.package_record.source_key.IsValid()) {
        return PackageResourceStagingStatus::InvalidPackageRecord;
    }

    if (request.package_record.archive_byte_size == 0ULL) {
        return PackageResourceStagingStatus::InvalidPackageRecord;
    }

    if (!request.expected_type.IsValid()) {
        return PackageResourceStagingStatus::InvalidArgument;
    }

    if (request.package_record.type.value != request.expected_type.value) {
        return PackageResourceStagingStatus::TypeMismatch;
    }

    if (ByteRangeOverflows(request.package_record.archive_byte_offset, request.package_record.archive_byte_size)) {
        return PackageResourceStagingStatus::ByteRangeOutOfBounds;
    }

    return PackageResourceStagingStatus::Success;
}

bool PackageResourceStagingQueue::HasRequestId(std::uint64_t request_id) const {
    for (const PendingRecord &pending_record : pending_records_) {
        if (!pending_record.is_active) {
            continue;
        }

        if (pending_record.request_id == request_id) {
            return true;
        }
    }

    for (std::uint32_t index = 0U; index < snapshot_.completion_count; ++index) {
        if (completions_[index].request_id == request_id) {
            return true;
        }
    }

    return false;
}

PackageResourceStagingQueue::PendingRecord *PackageResourceStagingQueue::FindFreePendingRecord() {
    std::uint32_t index = 0U;
    for (PendingRecord &pending_record : pending_records_) {
        if (index >= snapshot_.request_capacity) {
            return nullptr;
        }

        if (!pending_record.is_active) {
            return &pending_record;
        }

        ++index;
    }

    return nullptr;
}

PackageResourceStagingQueue::PendingRecord *PackageResourceStagingQueue::FindPendingRecord(
    std::uint64_t request_id) {
    std::uint32_t index = 0U;
    for (PendingRecord &pending_record : pending_records_) {
        if (index >= snapshot_.request_capacity) {
            return nullptr;
        }

        if (!pending_record.is_active) {
            ++index;
            continue;
        }

        if (pending_record.request_id == request_id) {
            return &pending_record;
        }

        ++index;
    }

    return nullptr;
}

void PackageResourceStagingQueue::StorePendingRecord(const PackageResourceStagingRequest &request) {
    PendingRecord *pending_record = FindFreePendingRecord();
    if (pending_record == nullptr) {
        return;
    }

    pending_record->is_active = true;
    pending_record->package_record = request.package_record;
    pending_record->resource = request.resource;
    pending_record->expected_type = request.expected_type;
    pending_record->request_id = request.request_id;
    ++snapshot_.pending_count;
    UpdateMaxPendingCount();
}

bool PackageResourceStagingQueue::AppendCompletion(const PackageResourceStagingCompletion &completion) {
    if (snapshot_.completion_count >= snapshot_.completion_capacity) {
        return false;
    }

    completions_[snapshot_.completion_count] = completion;
    ++snapshot_.completion_count;
    UpdateMaxCompletionCount();
    return true;
}

void PackageResourceStagingQueue::RemovePendingRecord(PendingRecord &pending_record) {
    pending_record = PendingRecord{};
    if (snapshot_.pending_count == 0U) {
        return;
    }

    --snapshot_.pending_count;
}

void PackageResourceStagingQueue::UpdateMaxPendingCount() {
    if (snapshot_.pending_count > snapshot_.max_pending_count) {
        snapshot_.max_pending_count = snapshot_.pending_count;
    }
}

void PackageResourceStagingQueue::UpdateMaxCompletionCount() {
    if (snapshot_.completion_count > snapshot_.max_completion_count) {
        snapshot_.max_completion_count = snapshot_.completion_count;
    }
}
}
