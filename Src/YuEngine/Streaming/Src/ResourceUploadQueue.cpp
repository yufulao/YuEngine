// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Src/ResourceUploadQueue.cpp

#include "YuEngine/Streaming/ResourceUploadQueue.h"

#include <cstdint>
#include <limits>
#include <span>

#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Rhi/IRhiDevice.h"

namespace yuengine::streaming {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}

bool ByteRangeOverflows(std::uint32_t byte_offset, std::uint32_t byte_size) {
    const std::uint64_t end_offset = static_cast<std::uint64_t>(byte_offset) + static_cast<std::uint64_t>(byte_size);
    return end_offset > static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max());
}

bool ByteRangeExceedsSize(std::uint32_t byte_offset, std::uint32_t byte_size, std::size_t byte_count) {
    const std::uint64_t end_offset = static_cast<std::uint64_t>(byte_offset) + static_cast<std::uint64_t>(byte_size);
    const std::uint64_t available_byte_count = static_cast<std::uint64_t>(byte_count);
    return end_offset > available_byte_count;
}

bool ResourceHandlesMatch(resource::ResourceHandle left, resource::ResourceHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool IsBufferUploadKind(ResourceUploadKind upload_kind) {
    if (upload_kind == ResourceUploadKind::CreateBuffer) {
        return true;
    }

    return upload_kind == ResourceUploadKind::UpdateBuffer;
}

bool IsTextureUploadKind(ResourceUploadKind upload_kind) {
    if (upload_kind == ResourceUploadKind::CreateTexture) {
        return true;
    }

    return upload_kind == ResourceUploadKind::UpdateTexture;
}
}

ResourceUploadQueue::ResourceUploadQueue()
    : ResourceUploadQueue(ResourceUploadQueueDesc{}) {
}

ResourceUploadQueue::ResourceUploadQueue(ResourceUploadQueueDesc desc)
    : pending_records_{},
      completions_{},
      snapshot_{
          ClampCapacity(desc.request_capacity, MAX_RESOURCE_UPLOAD_REQUEST_COUNT),
          ClampCapacity(desc.completion_capacity, MAX_RESOURCE_UPLOAD_COMPLETION_COUNT),
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
          ResourceUploadStatus::Success,
          resource::ResourceStatus::Success,
          rhi::RhiStatus::Success,
          memory::MemoryAccountingStatus::ExplicitlyTrackedOnly} {
}

ResourceUploadStatus ResourceUploadQueue::Submit(const ResourceUploadRequest &request) {
    const ResourceUploadStatus request_status = ValidateRequest(request);
    if (request_status != ResourceUploadStatus::Success) {
        return RecordRejected(request_status);
    }

    const resource::ResourceStatus resource_status =
        request.resource_registry->ValidateAcquire(request.resource, request.expected_type, 0U);
    if (resource_status == resource::ResourceStatus::TypeMismatch) {
        return RecordRejected(ResourceUploadStatus::TypeMismatch, resource_status);
    }

    if (resource_status != resource::ResourceStatus::Success) {
        return RecordRejected(ResourceUploadStatus::ResourceValidationFailed, resource_status);
    }

    if (HasUploadId(request.upload_id)) {
        ++snapshot_.duplicate_upload_count;
        return RecordRejected(ResourceUploadStatus::DuplicateUploadId);
    }

    if (snapshot_.pending_count >= snapshot_.request_capacity) {
        return RecordRejected(ResourceUploadStatus::QueueFull);
    }

    StorePendingRecord(request);
    ++snapshot_.submitted_count;
    snapshot_.last_status = ResourceUploadStatus::Queued;
    snapshot_.last_resource_status = resource::ResourceStatus::Success;
    snapshot_.last_rhi_status = rhi::RhiStatus::Success;
    return ResourceUploadStatus::Queued;
}

ResourceUploadStatus ResourceUploadQueue::ProcessNext() {
    PendingRecord *pending_record = FindOldestPendingRecord();
    if (pending_record == nullptr) {
        return ResourceUploadStatus::InvalidArgument;
    }

    if (snapshot_.completion_count >= snapshot_.completion_capacity) {
        return RecordCompletionOverflow();
    }

    return ProcessPendingRecord(*pending_record);
}

ResourceUploadStatus ResourceUploadQueue::DrainCompletions(
    ResourceUploadCompletion *output_completions,
    std::uint32_t output_capacity,
    std::uint32_t *written_count) {
    if (written_count == nullptr) {
        return ResourceUploadStatus::InvalidArgument;
    }

    *written_count = 0U;

    if (snapshot_.completion_count == 0U) {
        snapshot_.last_status = ResourceUploadStatus::Success;
        return ResourceUploadStatus::Success;
    }

    if (output_completions == nullptr) {
        return ResourceUploadStatus::InvalidArgument;
    }

    if (output_capacity < snapshot_.completion_count) {
        return ResourceUploadStatus::CompletionQueueFull;
    }

    for (std::uint32_t index = 0U; index < snapshot_.completion_count; ++index) {
        output_completions[index] = completions_[index];
        ++(*written_count);
        completions_[index] = ResourceUploadCompletion{};
    }

    snapshot_.completion_count = 0U;
    snapshot_.last_status = ResourceUploadStatus::Success;
    return ResourceUploadStatus::Success;
}

ResourceUploadSnapshot ResourceUploadQueue::Snapshot() const {
    return snapshot_;
}

ResourceUploadStatus ResourceUploadQueue::RecordRejected(ResourceUploadStatus status) {
    ++snapshot_.rejected_count;
    snapshot_.last_status = status;
    return status;
}

ResourceUploadStatus ResourceUploadQueue::RecordRejected(
    ResourceUploadStatus status,
    resource::ResourceStatus resource_status) {
    ++snapshot_.rejected_count;
    snapshot_.last_status = status;
    snapshot_.last_resource_status = resource_status;
    return status;
}

ResourceUploadStatus ResourceUploadQueue::RecordCompletionOverflow() {
    snapshot_.last_status = ResourceUploadStatus::CompletionQueueFull;
    return ResourceUploadStatus::CompletionQueueFull;
}

ResourceUploadStatus ResourceUploadQueue::ValidateRequest(const ResourceUploadRequest &request) const {
    if (request.resource_registry == nullptr) {
        return ResourceUploadStatus::InvalidArgument;
    }

    if (request.rhi_device == nullptr) {
        return ResourceUploadStatus::InvalidArgument;
    }

    if (!request.expected_type.IsValid()) {
        return ResourceUploadStatus::InvalidArgument;
    }

    if (request.staging_completion.status != PackageResourceStagingStatus::Success) {
        return ResourceUploadStatus::InvalidStagingCompletion;
    }

    if (!ResourceHandlesMatch(request.resource, request.staging_completion.resource)) {
        return ResourceUploadStatus::InvalidStagingCompletion;
    }

    if (request.expected_type.value != request.staging_completion.expected_type.value) {
        return ResourceUploadStatus::TypeMismatch;
    }

    if (request.upload_byte_count == 0U) {
        return ResourceUploadStatus::EmptyUploadBytes;
    }

    if (request.staged_bytes.data() == nullptr) {
        return ResourceUploadStatus::InvalidArgument;
    }

    if (ByteRangeOverflows(request.upload_byte_offset, request.upload_byte_count)) {
        return ResourceUploadStatus::ByteRangeOutOfBounds;
    }

    if (ByteRangeExceedsSize(
            request.upload_byte_offset,
            request.upload_byte_count,
            request.staging_completion.staged_byte_count)) {
        return ResourceUploadStatus::ByteRangeOutOfBounds;
    }

    if (ByteRangeExceedsSize(
            request.upload_byte_offset,
            request.upload_byte_count,
            request.staged_bytes.size())) {
        return ResourceUploadStatus::ByteRangeOutOfBounds;
    }

    if (ByteRangeExceedsSize(
            request.staging_completion.staged_byte_offset,
            request.staging_completion.staged_byte_count,
            request.staging_completion.file_byte_count)) {
        return ResourceUploadStatus::InvalidStagingCompletion;
    }

    if (!IsBufferUploadKind(request.upload_kind) && !IsTextureUploadKind(request.upload_kind)) {
        return ResourceUploadStatus::UnsupportedUploadKind;
    }

    if (request.upload_kind == ResourceUploadKind::CreateBuffer && request.output_buffer_handle == nullptr) {
        return ResourceUploadStatus::InvalidArgument;
    }

    if (request.upload_kind == ResourceUploadKind::UpdateBuffer && request.output_fence == nullptr) {
        return ResourceUploadStatus::InvalidArgument;
    }

    if (request.upload_kind == ResourceUploadKind::CreateTexture && request.output_texture_handle == nullptr) {
        return ResourceUploadStatus::InvalidArgument;
    }

    if (request.upload_kind == ResourceUploadKind::UpdateTexture && request.output_fence == nullptr) {
        return ResourceUploadStatus::InvalidArgument;
    }

    return ResourceUploadStatus::Success;
}

bool ResourceUploadQueue::HasUploadId(std::uint64_t upload_id) const {
    for (const PendingRecord &pending_record : pending_records_) {
        if (!pending_record.is_active) {
            continue;
        }

        if (pending_record.request.upload_id == upload_id) {
            return true;
        }
    }

    for (std::uint32_t index = 0U; index < snapshot_.completion_count; ++index) {
        if (completions_[index].upload_id == upload_id) {
            return true;
        }
    }

    return false;
}

ResourceUploadQueue::PendingRecord *ResourceUploadQueue::FindFreePendingRecord() {
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

ResourceUploadQueue::PendingRecord *ResourceUploadQueue::FindOldestPendingRecord() {
    std::uint32_t index = 0U;
    for (PendingRecord &pending_record : pending_records_) {
        if (index >= snapshot_.request_capacity) {
            return nullptr;
        }

        if (pending_record.is_active) {
            return &pending_record;
        }

        ++index;
    }

    return nullptr;
}

void ResourceUploadQueue::StorePendingRecord(const ResourceUploadRequest &request) {
    PendingRecord *pending_record = FindFreePendingRecord();
    if (pending_record == nullptr) {
        return;
    }

    pending_record->is_active = true;
    pending_record->request = request;
    ++snapshot_.pending_count;
    UpdateMaxPendingCount();
}

bool ResourceUploadQueue::AppendCompletion(const ResourceUploadCompletion &completion) {
    if (snapshot_.completion_count >= snapshot_.completion_capacity) {
        return false;
    }

    completions_[snapshot_.completion_count] = completion;
    ++snapshot_.completion_count;
    UpdateMaxCompletionCount();
    return true;
}

void ResourceUploadQueue::RemovePendingRecord(PendingRecord &pending_record) {
    pending_record = PendingRecord{};
    if (snapshot_.pending_count == 0U) {
        return;
    }

    --snapshot_.pending_count;
}

ResourceUploadCompletion ResourceUploadQueue::BuildBaseCompletion(const ResourceUploadRequest &request) const {
    ResourceUploadCompletion completion;
    completion.status = ResourceUploadStatus::Success;
    completion.upload_kind = request.upload_kind;
    completion.resource = request.resource;
    completion.expected_type = request.expected_type;
    completion.upload_id = request.upload_id;
    completion.staging_request_id = request.staging_completion.request_id;
    completion.upload_byte_count = request.upload_byte_count;
    return completion;
}

ResourceUploadStatus ResourceUploadQueue::ProcessPendingRecord(PendingRecord &pending_record) {
    ResourceUploadRequest &request = pending_record.request;
    ResourceUploadCompletion completion = BuildBaseCompletion(request);

    if (request.upload_kind == ResourceUploadKind::CreateBuffer) {
        return ProcessCreateBuffer(pending_record, request, completion);
    }

    if (request.upload_kind == ResourceUploadKind::UpdateBuffer) {
        return ProcessUpdateBuffer(pending_record, request, completion);
    }

    if (request.upload_kind == ResourceUploadKind::CreateTexture) {
        return ProcessCreateTexture(pending_record, request, completion);
    }

    if (request.upload_kind == ResourceUploadKind::UpdateTexture) {
        return ProcessUpdateTexture(pending_record, request, completion);
    }

    completion.status = ResourceUploadStatus::UnsupportedUploadKind;
    FinishFailedRecord(pending_record, completion);
    return completion.status;
}

ResourceUploadStatus ResourceUploadQueue::ProcessCreateBuffer(
    PendingRecord &pending_record,
    ResourceUploadRequest &request,
    ResourceUploadCompletion &completion) {
    const std::span<const std::uint8_t> bytes =
        request.staged_bytes.subspan(request.upload_byte_offset, request.upload_byte_count);
    rhi::RhiBufferHandle handle;
    const rhi::RhiStatus rhi_status = request.rhi_device->CreateBuffer(request.buffer_desc, bytes, handle);
    completion.rhi_status = rhi_status;
    completion.buffer_handle = handle;
    if (rhi_status != rhi::RhiStatus::Success) {
        completion.status = ResourceUploadStatus::RhiUploadFailed;
        FinishFailedRecord(pending_record, completion);
        return completion.status;
    }

    *request.output_buffer_handle = handle;
    FinishCompletedRecord(pending_record, completion);
    return completion.status;
}

ResourceUploadStatus ResourceUploadQueue::ProcessUpdateBuffer(
    PendingRecord &pending_record,
    ResourceUploadRequest &request,
    ResourceUploadCompletion &completion) {
    const std::span<const std::uint8_t> bytes =
        request.staged_bytes.subspan(request.upload_byte_offset, request.upload_byte_count);
    rhi::RhiFenceHandle fence;
    const rhi::RhiStatus rhi_status = request.rhi_device->UpdateBuffer(
        request.input_buffer_handle,
        bytes,
        fence,
        request.destination_byte_offset);
    completion.rhi_status = rhi_status;
    completion.fence = fence;
    if (rhi_status != rhi::RhiStatus::Success) {
        completion.status = ResourceUploadStatus::RhiUploadFailed;
        FinishFailedRecord(pending_record, completion);
        return completion.status;
    }

    *request.output_fence = fence;
    FinishCompletedRecord(pending_record, completion);
    return completion.status;
}

ResourceUploadStatus ResourceUploadQueue::ProcessCreateTexture(
    PendingRecord &pending_record,
    ResourceUploadRequest &request,
    ResourceUploadCompletion &completion) {
    const std::span<const std::uint8_t> bytes =
        request.staged_bytes.subspan(request.upload_byte_offset, request.upload_byte_count);
    rhi::RhiTextureHandle handle;
    const rhi::RhiStatus rhi_status = request.rhi_device->CreateTexture(request.texture_desc, bytes, handle);
    completion.rhi_status = rhi_status;
    completion.texture_handle = handle;
    if (rhi_status != rhi::RhiStatus::Success) {
        completion.status = ResourceUploadStatus::RhiUploadFailed;
        FinishFailedRecord(pending_record, completion);
        return completion.status;
    }

    *request.output_texture_handle = handle;
    FinishCompletedRecord(pending_record, completion);
    return completion.status;
}

ResourceUploadStatus ResourceUploadQueue::ProcessUpdateTexture(
    PendingRecord &pending_record,
    ResourceUploadRequest &request,
    ResourceUploadCompletion &completion) {
    const std::span<const std::uint8_t> bytes =
        request.staged_bytes.subspan(request.upload_byte_offset, request.upload_byte_count);
    rhi::RhiFenceHandle fence;
    const rhi::RhiStatus rhi_status = request.rhi_device->UpdateTexture(
        request.input_texture_handle,
        bytes,
        fence,
        request.destination_byte_offset);
    completion.rhi_status = rhi_status;
    completion.fence = fence;
    if (rhi_status != rhi::RhiStatus::Success) {
        completion.status = ResourceUploadStatus::RhiUploadFailed;
        FinishFailedRecord(pending_record, completion);
        return completion.status;
    }

    *request.output_fence = fence;
    FinishCompletedRecord(pending_record, completion);
    return completion.status;
}

void ResourceUploadQueue::FinishCompletedRecord(
    PendingRecord &pending_record,
    const ResourceUploadCompletion &completion) {
    AppendCompletion(completion);
    RemovePendingRecord(pending_record);
    ++snapshot_.completed_count;
    snapshot_.last_status = completion.status;
    snapshot_.last_resource_status = completion.resource_status;
    snapshot_.last_rhi_status = completion.rhi_status;
}

void ResourceUploadQueue::FinishFailedRecord(
    PendingRecord &pending_record,
    const ResourceUploadCompletion &completion) {
    AppendCompletion(completion);
    RemovePendingRecord(pending_record);
    ++snapshot_.failed_count;
    if (completion.rhi_status != rhi::RhiStatus::Success) {
        ++snapshot_.rhi_upload_failed_count;
    }

    snapshot_.last_status = completion.status;
    snapshot_.last_resource_status = completion.resource_status;
    snapshot_.last_rhi_status = completion.rhi_status;
}

void ResourceUploadQueue::UpdateMaxPendingCount() {
    if (snapshot_.pending_count > snapshot_.max_pending_count) {
        snapshot_.max_pending_count = snapshot_.pending_count;
    }
}

void ResourceUploadQueue::UpdateMaxCompletionCount() {
    if (snapshot_.completion_count > snapshot_.max_completion_count) {
        snapshot_.max_completion_count = snapshot_.completion_count;
    }
}
}
