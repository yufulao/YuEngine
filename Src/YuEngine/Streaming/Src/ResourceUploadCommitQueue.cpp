// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Src/ResourceUploadCommitQueue.cpp

#include "YuEngine/Streaming/ResourceUploadCommitQueue.h"

#include "YuEngine/Resource/ResourceLoadCommitRequest.h"
#include "YuEngine/Resource/ResourceRegistry.h"

namespace yuengine::streaming {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}

resource::ResourceLoadState LoadStateFromUploadStatus(ResourceUploadStatus status) {
    if (status == ResourceUploadStatus::Success) {
        return resource::ResourceLoadState::Uploaded;
    }

    return resource::ResourceLoadState::Failed;
}

bool IsTerminalUploadStatus(ResourceUploadStatus status) {
    if (status == ResourceUploadStatus::Queued) {
        return false;
    }

    return true;
}
}

ResourceUploadCommitQueue::ResourceUploadCommitQueue()
    : ResourceUploadCommitQueue(ResourceUploadCommitQueueDesc{}) {
}

ResourceUploadCommitQueue::ResourceUploadCommitQueue(ResourceUploadCommitQueueDesc desc)
    : pending_records_{},
      completions_{},
      snapshot_{
          ClampCapacity(desc.request_capacity, MAX_RESOURCE_UPLOAD_COMMIT_REQUEST_COUNT),
          ClampCapacity(desc.completion_capacity, MAX_RESOURCE_UPLOAD_COMMIT_COMPLETION_COUNT),
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
          ResourceUploadCommitStatus::Success,
          resource::ResourceLoadCommitStatus::Success,
          resource::ResourceLoadState::Unloaded,
          memory::MemoryAccountingStatus::ExplicitlyTrackedOnly},
      next_pending_sequence_(0U) {
}

ResourceUploadCommitStatus ResourceUploadCommitQueue::Submit(const ResourceUploadCommitRequest &request) {
    const ResourceUploadCommitStatus request_status = ValidateRequest(request);
    if (request_status != ResourceUploadCommitStatus::Success) {
        return RecordRejected(request_status);
    }

    if (HasCommitId(request.commit_id)) {
        ++snapshot_.duplicate_commit_count;
        return RecordRejected(ResourceUploadCommitStatus::DuplicateCommitId);
    }

    if (snapshot_.pending_count >= snapshot_.request_capacity) {
        return RecordRejected(ResourceUploadCommitStatus::QueueFull);
    }

    StorePendingRecord(request);
    ++snapshot_.submitted_count;
    snapshot_.last_status = ResourceUploadCommitStatus::Queued;
    snapshot_.last_resource_commit_status = resource::ResourceLoadCommitStatus::Success;
    snapshot_.last_load_state = resource::ResourceLoadState::Unloaded;
    return ResourceUploadCommitStatus::Queued;
}

ResourceUploadCommitStatus ResourceUploadCommitQueue::ProcessNext() {
    PendingRecord *pending_record = FindOldestPendingRecord();
    if (pending_record == nullptr) {
        return ResourceUploadCommitStatus::InvalidArgument;
    }

    if (snapshot_.completion_count >= snapshot_.completion_capacity) {
        return RecordCompletionOverflow();
    }

    return ProcessPendingRecord(*pending_record);
}

ResourceUploadCommitStatus ResourceUploadCommitQueue::DrainCompletions(
    ResourceUploadCommitCompletion *output_completions,
    std::uint32_t output_capacity,
    std::uint32_t *written_count) {
    if (written_count == nullptr) {
        return ResourceUploadCommitStatus::InvalidArgument;
    }

    *written_count = 0U;
    if (snapshot_.completion_count == 0U) {
        snapshot_.last_status = ResourceUploadCommitStatus::Success;
        return ResourceUploadCommitStatus::Success;
    }

    if (output_completions == nullptr) {
        return ResourceUploadCommitStatus::InvalidArgument;
    }

    if (output_capacity < snapshot_.completion_count) {
        return ResourceUploadCommitStatus::CompletionQueueFull;
    }

    for (std::uint32_t index = 0U; index < snapshot_.completion_count; ++index) {
        output_completions[index] = completions_[index];
        ++(*written_count);
        completions_[index] = ResourceUploadCommitCompletion{};
    }

    snapshot_.completion_count = 0U;
    snapshot_.last_status = ResourceUploadCommitStatus::Success;
    return ResourceUploadCommitStatus::Success;
}

ResourceUploadCommitSnapshot ResourceUploadCommitQueue::Snapshot() const {
    return snapshot_;
}

ResourceUploadCommitStatus ResourceUploadCommitQueue::RecordRejected(ResourceUploadCommitStatus status) {
    ++snapshot_.rejected_count;
    snapshot_.last_status = status;
    return status;
}

ResourceUploadCommitStatus ResourceUploadCommitQueue::RecordCompletionOverflow() {
    snapshot_.last_status = ResourceUploadCommitStatus::CompletionQueueFull;
    return ResourceUploadCommitStatus::CompletionQueueFull;
}

ResourceUploadCommitStatus ResourceUploadCommitQueue::ValidateRequest(
    const ResourceUploadCommitRequest &request) const {
    if (request.resource_registry == nullptr) {
        return ResourceUploadCommitStatus::InvalidArgument;
    }

    if (request.commit_id == 0U) {
        return ResourceUploadCommitStatus::InvalidArgument;
    }

    if (request.upload_completion.upload_id == 0U) {
        return ResourceUploadCommitStatus::InvalidArgument;
    }

    if (!request.upload_completion.expected_type.IsValid()) {
        return ResourceUploadCommitStatus::InvalidArgument;
    }

    if (!IsTerminalUploadStatus(request.upload_completion.status)) {
        return ResourceUploadCommitStatus::InvalidArgument;
    }

    return ResourceUploadCommitStatus::Success;
}

bool ResourceUploadCommitQueue::HasCommitId(std::uint64_t commit_id) const {
    for (const PendingRecord &pending_record : pending_records_) {
        if (!pending_record.is_active) {
            continue;
        }

        if (pending_record.request.commit_id == commit_id) {
            return true;
        }
    }

    for (std::uint32_t index = 0U; index < snapshot_.completion_count; ++index) {
        if (completions_[index].commit_id == commit_id) {
            return true;
        }
    }

    return false;
}

ResourceUploadCommitQueue::PendingRecord *ResourceUploadCommitQueue::FindFreePendingRecord() {
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

ResourceUploadCommitQueue::PendingRecord *ResourceUploadCommitQueue::FindOldestPendingRecord() {
    std::uint32_t index = 0U;
    PendingRecord *oldest_record = nullptr;
    for (PendingRecord &pending_record : pending_records_) {
        if (index >= snapshot_.request_capacity) {
            return oldest_record;
        }

        if (pending_record.is_active) {
            if (oldest_record == nullptr) {
                oldest_record = &pending_record;
            }

            if (pending_record.sequence_id < oldest_record->sequence_id) {
                oldest_record = &pending_record;
            }
        }

        ++index;
    }

    return oldest_record;
}

void ResourceUploadCommitQueue::StorePendingRecord(const ResourceUploadCommitRequest &request) {
    PendingRecord *pending_record = FindFreePendingRecord();
    if (pending_record == nullptr) {
        return;
    }

    pending_record->is_active = true;
    pending_record->sequence_id = next_pending_sequence_;
    pending_record->request = request;
    ++next_pending_sequence_;
    ++snapshot_.pending_count;
    UpdateMaxPendingCount();
}

bool ResourceUploadCommitQueue::AppendCompletion(const ResourceUploadCommitCompletion &completion) {
    if (snapshot_.completion_count >= snapshot_.completion_capacity) {
        return false;
    }

    completions_[snapshot_.completion_count] = completion;
    ++snapshot_.completion_count;
    UpdateMaxCompletionCount();
    return true;
}

void ResourceUploadCommitQueue::RemovePendingRecord(PendingRecord &pending_record) {
    pending_record = PendingRecord{};
    if (snapshot_.pending_count == 0U) {
        return;
    }

    --snapshot_.pending_count;
}

ResourceUploadCommitCompletion ResourceUploadCommitQueue::BuildBaseCompletion(
    const ResourceUploadCommitRequest &request) const {
    ResourceUploadCommitCompletion completion;
    completion.status = ResourceUploadCommitStatus::Success;
    completion.resource_commit_status = resource::ResourceLoadCommitStatus::Success;
    completion.load_state = LoadStateFromUploadStatus(request.upload_completion.status);
    completion.upload_status = request.upload_completion.status;
    completion.resource = request.upload_completion.resource;
    completion.expected_type = request.upload_completion.expected_type;
    completion.commit_id = request.commit_id;
    completion.upload_id = request.upload_completion.upload_id;
    completion.staging_request_id = request.upload_completion.staging_request_id;
    completion.upload_byte_count = request.upload_completion.upload_byte_count;
    completion.rhi_status = request.upload_completion.rhi_status;
    return completion;
}

ResourceUploadCommitStatus ResourceUploadCommitQueue::ProcessPendingRecord(PendingRecord &pending_record) {
    ResourceUploadCommitRequest &request = pending_record.request;
    ResourceUploadCommitCompletion completion = BuildBaseCompletion(request);
    resource::ResourceLoadCommitRequest load_request;
    load_request.resource = completion.resource;
    load_request.expected_type = completion.expected_type;
    load_request.load_state = completion.load_state;
    load_request.commit_id = completion.commit_id;
    load_request.upload_id = completion.upload_id;
    load_request.staging_request_id = completion.staging_request_id;
    load_request.upload_byte_count = completion.upload_byte_count;

    const resource::ResourceLoadCommitStatus resource_status =
        request.resource_registry->CommitUploadCompletion(load_request);
    completion.resource_commit_status = resource_status;
    if (resource_status != resource::ResourceLoadCommitStatus::Success) {
        completion.status = ResourceUploadCommitStatus::ResourceCommitFailed;
        FinishFailedRecord(pending_record, completion);
        return completion.status;
    }

    FinishCompletedRecord(pending_record, completion);
    return completion.status;
}

void ResourceUploadCommitQueue::FinishCompletedRecord(
    PendingRecord &pending_record,
    const ResourceUploadCommitCompletion &completion) {
    AppendCompletion(completion);
    RemovePendingRecord(pending_record);
    ++snapshot_.committed_count;
    if (completion.load_state == resource::ResourceLoadState::Failed) {
        ++snapshot_.failed_upload_commit_count;
    }

    snapshot_.last_status = completion.status;
    snapshot_.last_resource_commit_status = completion.resource_commit_status;
    snapshot_.last_load_state = completion.load_state;
}

void ResourceUploadCommitQueue::FinishFailedRecord(
    PendingRecord &pending_record,
    const ResourceUploadCommitCompletion &completion) {
    AppendCompletion(completion);
    RemovePendingRecord(pending_record);
    ++snapshot_.resource_commit_failed_count;
    snapshot_.last_status = completion.status;
    snapshot_.last_resource_commit_status = completion.resource_commit_status;
    snapshot_.last_load_state = completion.load_state;
}

void ResourceUploadCommitQueue::UpdateMaxPendingCount() {
    if (snapshot_.pending_count > snapshot_.max_pending_count) {
        snapshot_.max_pending_count = snapshot_.pending_count;
    }
}

void ResourceUploadCommitQueue::UpdateMaxCompletionCount() {
    if (snapshot_.completion_count > snapshot_.max_completion_count) {
        snapshot_.max_completion_count = snapshot_.completion_count;
    }
}
}
