// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Src/PackageResourceStagingQueue.cpp

#include "YuEngine/Streaming/PackageResourceStagingQueue.h"

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

    file::AsyncFileReadRequest file_request = request.file_request;
    file_request.read_request.use_range = true;
    file_request.read_request.range_byte_offset = request.package_record.archive_byte_offset;
    file_request.read_request.range_byte_size = request.package_record.archive_byte_size;
    file_request.request_index = request.request_id;
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

    *written_count = 0U;

    if (snapshot_.completion_count == 0U) {
        snapshot_.last_status = PackageResourceStagingStatus::Success;
        return PackageResourceStagingStatus::Success;
    }

    if (output_completions == nullptr) {
        return PackageResourceStagingStatus::InvalidArgument;
    }

    if (output_capacity < snapshot_.completion_count) {
        return PackageResourceStagingStatus::CompletionQueueFull;
    }

    PackageResourceStagingStatus last_status = PackageResourceStagingStatus::Success;
    for (std::uint32_t index = 0U; index < snapshot_.completion_count; ++index) {
        if (completions_[index].status != PackageResourceStagingStatus::Success) {
            last_status = completions_[index].status;
        }

        output_completions[index] = completions_[index];
        ++(*written_count);
        completions_[index] = PackageResourceStagingCompletion{};
    }

    snapshot_.completion_count = 0U;
    snapshot_.last_status = last_status;
    return PackageResourceStagingStatus::Success;
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
