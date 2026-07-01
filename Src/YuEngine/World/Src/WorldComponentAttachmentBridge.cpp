// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldComponentAttachmentBridge.cpp

#include "YuEngine/World/WorldComponentAttachmentBridge.h"

#include "YuEngine/Memory/MemoryAccountingStatus.h"

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

WorldComponentAttachmentBridge::WorldComponentAttachmentBridge(WorldComponentAttachmentBridgeDesc desc)
    : attachments_{},
      snapshot_{
          ClampCapacity(desc.attachment_capacity, MAX_WORLD_OBJECT_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          WorldComponentAttachmentStatus::Success} {
    if (desc.attachment_capacity == 0U) {
        snapshot_.last_status = WorldComponentAttachmentStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldComponentAttachmentResult WorldComponentAttachmentBridge::Add(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id) {
    const WorldComponentAttachmentStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentStatus::Success) {
        return RecordFailureResult(capacity_status);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailureResult(WorldComponentAttachmentStatus::InvalidWorldObjectId);
    }

    if (!component_type_id.IsValid()) {
        return RecordFailureResult(WorldComponentAttachmentStatus::InvalidComponentTypeId);
    }

    if (!component_slot_id.IsValid()) {
        return RecordFailureResult(WorldComponentAttachmentStatus::InvalidComponentSlotId);
    }

    if (FindAttachment(world_object_id, component_type_id) != nullptr) {
        return RecordDuplicateFailureResult();
    }

    if (snapshot_.active_attachment_count >= snapshot_.attachment_capacity) {
        return RecordFailureResult(WorldComponentAttachmentStatus::CapacityExceeded);
    }

    WorldComponentAttachment *attachment = FindFreeAttachment();
    if (attachment == nullptr) {
        return RecordFailureResult(WorldComponentAttachmentStatus::CapacityExceeded);
    }

    attachment->world_object_id = world_object_id;
    attachment->component_type_id = component_type_id;
    attachment->component_slot_id = component_slot_id;
    attachment->is_attached = true;
    ++snapshot_.active_attachment_count;
    ++snapshot_.added_attachment_count;
    RecordSuccess();
    return WorldComponentAttachmentResult::Success(world_object_id, component_type_id, component_slot_id);
}

WorldComponentAttachmentResult WorldComponentAttachmentBridge::Query(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id) {
    const WorldComponentAttachmentStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentStatus::Success) {
        return RecordFailureResult(capacity_status);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailureResult(WorldComponentAttachmentStatus::InvalidWorldObjectId);
    }

    if (!component_type_id.IsValid()) {
        return RecordFailureResult(WorldComponentAttachmentStatus::InvalidComponentTypeId);
    }

    ++snapshot_.query_count;
    const WorldComponentAttachment *attachment = FindAttachment(world_object_id, component_type_id);
    if (attachment == nullptr) {
        return RecordFailureResult(WorldComponentAttachmentStatus::AttachmentNotFound);
    }

    RecordSuccess();
    return WorldComponentAttachmentResult::Success(
        attachment->world_object_id,
        attachment->component_type_id,
        attachment->component_slot_id);
}

WorldComponentAttachmentResult WorldComponentAttachmentBridge::Lookup(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentAttachment *output_attachment) {
    const WorldComponentAttachmentStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentStatus::Success) {
        return RecordFailureResult(capacity_status);
    }

    WorldComponentAttachmentQueryDesc query_desc{};
    query_desc.world_object_id = world_object_id;
    query_desc.component_type_id = component_type_id;
    const WorldComponentAttachmentStatus query_status = ValidateQueryDesc(query_desc);
    if (query_status != WorldComponentAttachmentStatus::Success) {
        return RecordFailureResult(query_status);
    }

    if (output_attachment == nullptr) {
        return RecordFailureResult(WorldComponentAttachmentStatus::InvalidOutputBuffer);
    }

    ++snapshot_.query_count;
    const WorldComponentAttachment *attachment = FindAttachment(world_object_id, component_type_id);
    if (attachment == nullptr) {
        return RecordFailureResult(WorldComponentAttachmentStatus::AttachmentNotFound);
    }

    *output_attachment = *attachment;
    RecordSuccess();
    return WorldComponentAttachmentResult::Success(
        attachment->world_object_id,
        attachment->component_type_id,
        attachment->component_slot_id);
}

WorldComponentAttachmentResult WorldComponentAttachmentBridge::EnumerateType(
    WorldComponentTypeId component_type_id,
    WorldComponentAttachment *output_attachments,
    std::uint32_t output_capacity) {
    const WorldComponentAttachmentStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentStatus::Success) {
        return RecordBatchFailureResult(capacity_status, 0U, 0U);
    }

    if (!component_type_id.IsValid()) {
        return RecordBatchFailureResult(WorldComponentAttachmentStatus::InvalidComponentTypeId, 0U, 0U);
    }

    std::uint32_t required_record_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.attachment_capacity; ++index) {
        const WorldComponentAttachment &attachment = attachments_[index];
        if (!attachment.is_attached) {
            continue;
        }

        if (attachment.component_type_id.value != component_type_id.value) {
            continue;
        }

        ++required_record_count;
    }

    if (required_record_count == 0U) {
        ++snapshot_.query_count;
        RecordSuccess();
        return WorldComponentAttachmentResult::BatchSuccess(0U, 0U);
    }

    if (output_attachments == nullptr) {
        return RecordBatchFailureResult(
            WorldComponentAttachmentStatus::InvalidOutputBuffer,
            required_record_count,
            0U);
    }

    if (required_record_count > output_capacity) {
        return RecordBatchFailureResult(
            WorldComponentAttachmentStatus::CapacityExceeded,
            required_record_count,
            0U);
    }

    std::uint32_t written_record_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.attachment_capacity; ++index) {
        const WorldComponentAttachment &attachment = attachments_[index];
        if (!attachment.is_attached) {
            continue;
        }

        if (attachment.component_type_id.value != component_type_id.value) {
            continue;
        }

        output_attachments[written_record_count] = attachment;
        ++written_record_count;
    }

    ++snapshot_.query_count;
    RecordSuccess();
    return WorldComponentAttachmentResult::BatchSuccess(required_record_count, written_record_count);
}

WorldComponentAttachmentStatus WorldComponentAttachmentBridge::CountType(
    WorldComponentTypeId component_type_id,
    std::uint32_t *output_count) {
    const WorldComponentAttachmentStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (!component_type_id.IsValid()) {
        return RecordFailure(WorldComponentAttachmentStatus::InvalidComponentTypeId);
    }

    if (output_count == nullptr) {
        return RecordFailure(WorldComponentAttachmentStatus::InvalidOutputBuffer);
    }

    std::uint32_t matched_record_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.attachment_capacity; ++index) {
        const WorldComponentAttachment &attachment = attachments_[index];
        if (!attachment.is_attached) {
            continue;
        }

        if (attachment.component_type_id.value != component_type_id.value) {
            continue;
        }

        ++matched_record_count;
    }

    *output_count = matched_record_count;
    RecordSuccess();
    return WorldComponentAttachmentStatus::Success;
}

WorldComponentAttachmentResult WorldComponentAttachmentBridge::CountTypes(
    const WorldComponentTypeId *component_type_ids,
    std::uint32_t component_type_count,
    std::uint32_t *output_counts,
    std::uint32_t output_capacity) {
    const WorldComponentAttachmentStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentStatus::Success) {
        return RecordBatchFailureResult(capacity_status, 0U, 0U);
    }

    if (component_type_count == 0U) {
        RecordSuccess();
        return WorldComponentAttachmentResult::BatchSuccess(0U, 0U);
    }

    if (component_type_ids == nullptr) {
        return RecordBatchFailureResult(WorldComponentAttachmentStatus::InvalidComponentTypeId, 0U, 0U);
    }

    for (std::uint32_t type_index = 0U; type_index < component_type_count; ++type_index) {
        const WorldComponentTypeId component_type_id = component_type_ids[type_index];
        if (!component_type_id.IsValid()) {
            return RecordBatchFailureResult(WorldComponentAttachmentStatus::InvalidComponentTypeId, 0U, 0U);
        }
    }

    if (output_counts == nullptr) {
        return RecordBatchFailureResult(
            WorldComponentAttachmentStatus::InvalidOutputBuffer,
            component_type_count,
            0U);
    }

    if (component_type_count > output_capacity) {
        return RecordBatchFailureResult(
            WorldComponentAttachmentStatus::CapacityExceeded,
            component_type_count,
            0U);
    }

    for (std::uint32_t type_index = 0U; type_index < component_type_count; ++type_index) {
        const WorldComponentTypeId component_type_id = component_type_ids[type_index];
        std::uint32_t matched_record_count = 0U;
        for (std::uint32_t attachment_index = 0U; attachment_index < snapshot_.attachment_capacity; ++attachment_index) {
            const WorldComponentAttachment &attachment = attachments_[attachment_index];
            if (!attachment.is_attached) {
                continue;
            }

            if (attachment.component_type_id.value != component_type_id.value) {
                continue;
            }

            ++matched_record_count;
        }

        output_counts[type_index] = matched_record_count;
    }

    RecordSuccess();
    return WorldComponentAttachmentResult::BatchSuccess(component_type_count, component_type_count);
}

WorldComponentAttachmentResult WorldComponentAttachmentBridge::EnumerateTypes(
    const WorldComponentTypeId *component_type_ids,
    std::uint32_t component_type_count,
    WorldComponentAttachment *output_attachments,
    std::uint32_t output_capacity) {
    const WorldComponentAttachmentStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentStatus::Success) {
        return RecordBatchFailureResult(capacity_status, 0U, 0U);
    }

    if (component_type_count == 0U) {
        RecordSuccess();
        return WorldComponentAttachmentResult::BatchSuccess(0U, 0U);
    }

    if (component_type_ids == nullptr) {
        return RecordBatchFailureResult(WorldComponentAttachmentStatus::InvalidComponentTypeId, 0U, 0U);
    }

    for (std::uint32_t type_index = 0U; type_index < component_type_count; ++type_index) {
        const WorldComponentTypeId component_type_id = component_type_ids[type_index];
        if (!component_type_id.IsValid()) {
            return RecordBatchFailureResult(WorldComponentAttachmentStatus::InvalidComponentTypeId, 0U, 0U);
        }
    }

    std::uint32_t required_record_count = 0U;
    for (std::uint32_t type_index = 0U; type_index < component_type_count; ++type_index) {
        const WorldComponentTypeId component_type_id = component_type_ids[type_index];
        for (std::uint32_t attachment_index = 0U; attachment_index < snapshot_.attachment_capacity; ++attachment_index) {
            const WorldComponentAttachment &attachment = attachments_[attachment_index];
            if (!attachment.is_attached) {
                continue;
            }

            if (attachment.component_type_id.value != component_type_id.value) {
                continue;
            }

            ++required_record_count;
        }
    }

    if (required_record_count == 0U) {
        snapshot_.query_count += component_type_count;
        RecordSuccess();
        return WorldComponentAttachmentResult::BatchSuccess(0U, 0U);
    }

    if (output_attachments == nullptr) {
        return RecordBatchFailureResult(
            WorldComponentAttachmentStatus::InvalidOutputBuffer,
            required_record_count,
            0U);
    }

    if (required_record_count > output_capacity) {
        return RecordBatchFailureResult(
            WorldComponentAttachmentStatus::CapacityExceeded,
            required_record_count,
            0U);
    }

    std::uint32_t written_record_count = 0U;
    for (std::uint32_t type_index = 0U; type_index < component_type_count; ++type_index) {
        const WorldComponentTypeId component_type_id = component_type_ids[type_index];
        for (std::uint32_t attachment_index = 0U; attachment_index < snapshot_.attachment_capacity; ++attachment_index) {
            const WorldComponentAttachment &attachment = attachments_[attachment_index];
            if (!attachment.is_attached) {
                continue;
            }

            if (attachment.component_type_id.value != component_type_id.value) {
                continue;
            }

            output_attachments[written_record_count] = attachment;
            ++written_record_count;
        }
    }

    snapshot_.query_count += component_type_count;
    RecordSuccess();
    return WorldComponentAttachmentResult::BatchSuccess(required_record_count, written_record_count);
}

WorldComponentAttachmentResult WorldComponentAttachmentBridge::QueryBatch(
    const WorldComponentAttachmentQueryDesc *query_descs,
    std::uint32_t query_count,
    WorldComponentAttachment *output_attachments,
    std::uint32_t output_capacity) {
    const WorldComponentAttachmentStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentStatus::Success) {
        return RecordBatchFailureResult(capacity_status, 0U, 0U);
    }

    if (query_count == 0U) {
        RecordSuccess();
        return WorldComponentAttachmentResult::BatchSuccess(0U, 0U);
    }

    if (query_descs == nullptr) {
        return RecordBatchFailureResult(WorldComponentAttachmentStatus::InvalidWorldObjectId, query_count, 0U);
    }

    if (output_attachments == nullptr) {
        return RecordBatchFailureResult(WorldComponentAttachmentStatus::InvalidOutputBuffer, query_count, 0U);
    }

    if (query_count > output_capacity) {
        return RecordBatchFailureResult(WorldComponentAttachmentStatus::CapacityExceeded, query_count, 0U);
    }

    for (std::uint32_t index = 0U; index < query_count; ++index) {
        const WorldComponentAttachmentQueryDesc &query_desc = query_descs[index];
        const WorldComponentAttachmentStatus query_status = ValidateQueryDesc(query_desc);
        if (query_status != WorldComponentAttachmentStatus::Success) {
            return RecordBatchFailureResult(query_status, query_count, 0U);
        }
    }

    for (std::uint32_t index = 0U; index < query_count; ++index) {
        const WorldComponentAttachmentQueryDesc &query_desc = query_descs[index];
        const WorldComponentAttachment *attachment = FindAttachment(
            query_desc.world_object_id,
            query_desc.component_type_id);
        if (attachment == nullptr) {
            return RecordBatchFailureResult(WorldComponentAttachmentStatus::AttachmentNotFound, query_count, 0U);
        }
    }

    for (std::uint32_t index = 0U; index < query_count; ++index) {
        const WorldComponentAttachmentQueryDesc &query_desc = query_descs[index];
        const WorldComponentAttachment *attachment = FindAttachment(
            query_desc.world_object_id,
            query_desc.component_type_id);
        if (attachment == nullptr) {
            return RecordBatchFailureResult(WorldComponentAttachmentStatus::AttachmentNotFound, query_count, 0U);
        }

        output_attachments[index] = *attachment;
    }

    snapshot_.query_count += query_count;
    RecordSuccess();
    return WorldComponentAttachmentResult::BatchSuccess(query_count, query_count);
}

WorldComponentAttachmentStatus WorldComponentAttachmentBridge::Remove(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id) {
    const WorldComponentAttachmentStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailure(WorldComponentAttachmentStatus::InvalidWorldObjectId);
    }

    if (!component_type_id.IsValid()) {
        return RecordFailure(WorldComponentAttachmentStatus::InvalidComponentTypeId);
    }

    WorldComponentAttachment *attachment = FindAttachment(world_object_id, component_type_id);
    if (attachment == nullptr) {
        return RecordFailure(WorldComponentAttachmentStatus::AttachmentNotFound);
    }

    ClearAttachment(*attachment);
    ++snapshot_.removed_attachment_count;
    RecountActiveAttachments();
    RecordSuccess();
    return WorldComponentAttachmentStatus::Success;
}

WorldComponentAttachmentStatus WorldComponentAttachmentBridge::Clear() {
    const WorldComponentAttachmentStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentStatus::Success) {
        return RecordFailure(capacity_status);
    }

    for (std::uint32_t index = 0U; index < snapshot_.attachment_capacity; ++index) {
        WorldComponentAttachment &attachment = attachments_[index];
        if (!attachment.is_attached) {
            continue;
        }

        ClearAttachment(attachment);
        ++snapshot_.cleared_attachment_count;
    }

    RecountActiveAttachments();
    RecordSuccess();
    return WorldComponentAttachmentStatus::Success;
}

WorldComponentAttachmentSnapshot WorldComponentAttachmentBridge::Snapshot() const {
    return snapshot_;
}

WorldComponentAttachmentStatus WorldComponentAttachmentBridge::ValidateRestoreDestination(
    std::uint32_t required_attachment_count) const {
    const WorldComponentAttachmentStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentStatus::Success) {
        return capacity_status;
    }

    if (snapshot_.active_attachment_count > 0U) {
        return WorldComponentAttachmentStatus::DuplicateAttachment;
    }

    if (required_attachment_count > snapshot_.attachment_capacity) {
        return WorldComponentAttachmentStatus::CapacityExceeded;
    }

    return WorldComponentAttachmentStatus::Success;
}

std::uint32_t WorldComponentAttachmentBridge::ExportAttachments(
    WorldComponentAttachment *output_attachments,
    std::uint32_t output_capacity) const {
    std::uint32_t exported_attachment_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.attachment_capacity; ++index) {
        const WorldComponentAttachment &attachment = attachments_[index];
        if (!attachment.is_attached) {
            continue;
        }

        if ((output_attachments != nullptr) && (exported_attachment_count < output_capacity)) {
            output_attachments[exported_attachment_count] = attachment;
        }

        ++exported_attachment_count;
    }

    return exported_attachment_count;
}

WorldComponentAttachmentExportResult WorldComponentAttachmentBridge::ExportAttachmentsChecked(
    WorldComponentAttachment *output_attachments,
    std::uint32_t output_capacity) const {
    WorldComponentAttachmentExportResult result{};
    const WorldComponentAttachmentStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentStatus::Success) {
        result.status = capacity_status;
        return result;
    }

    result.required_attachment_count = snapshot_.active_attachment_count;
    if (result.required_attachment_count > output_capacity) {
        result.status = WorldComponentAttachmentStatus::CapacityExceeded;
        return result;
    }

    if ((result.required_attachment_count > 0U) && (output_attachments == nullptr)) {
        result.status = WorldComponentAttachmentStatus::CapacityExceeded;
        return result;
    }

    result.exported_attachment_count = ExportAttachments(output_attachments, output_capacity);
    return result;
}

WorldComponentAttachmentResult WorldComponentAttachmentBridge::RecordFailureResult(
    WorldComponentAttachmentStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return WorldComponentAttachmentResult::Failure(status);
}

WorldComponentAttachmentResult WorldComponentAttachmentBridge::RecordBatchFailureResult(
    WorldComponentAttachmentStatus status,
    std::uint32_t required_record_count,
    std::uint32_t written_record_count) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return WorldComponentAttachmentResult::BatchFailure(
        status,
        required_record_count,
        written_record_count);
}

WorldComponentAttachmentResult WorldComponentAttachmentBridge::RecordDuplicateFailureResult() {
    ++snapshot_.duplicate_rejection_count;
    return RecordFailureResult(WorldComponentAttachmentStatus::DuplicateAttachment);
}

WorldComponentAttachmentStatus WorldComponentAttachmentBridge::RecordFailure(
    WorldComponentAttachmentStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return status;
}

void WorldComponentAttachmentBridge::RecordSuccess() {
    snapshot_.last_status = WorldComponentAttachmentStatus::Success;
}

WorldComponentAttachmentStatus WorldComponentAttachmentBridge::ValidateBridgeCapacity() const {
    if (snapshot_.attachment_capacity == 0U) {
        return WorldComponentAttachmentStatus::InvalidBridgeCapacity;
    }

    return WorldComponentAttachmentStatus::Success;
}

WorldComponentAttachmentStatus WorldComponentAttachmentBridge::ValidateQueryDesc(
    const WorldComponentAttachmentQueryDesc &query_desc) const {
    if (!query_desc.world_object_id.IsValid()) {
        return WorldComponentAttachmentStatus::InvalidWorldObjectId;
    }

    if (!query_desc.component_type_id.IsValid()) {
        return WorldComponentAttachmentStatus::InvalidComponentTypeId;
    }

    return WorldComponentAttachmentStatus::Success;
}

WorldComponentAttachment *WorldComponentAttachmentBridge::FindAttachment(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id) {
    for (std::uint32_t index = 0U; index < snapshot_.attachment_capacity; ++index) {
        WorldComponentAttachment &attachment = attachments_[index];
        if (!attachment.is_attached) {
            continue;
        }

        if (attachment.world_object_id.value != world_object_id.value) {
            continue;
        }

        if (attachment.component_type_id.value == component_type_id.value) {
            return &attachment;
        }
    }

    return nullptr;
}

const WorldComponentAttachment *WorldComponentAttachmentBridge::FindAttachment(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id) const {
    for (std::uint32_t index = 0U; index < snapshot_.attachment_capacity; ++index) {
        const WorldComponentAttachment &attachment = attachments_[index];
        if (!attachment.is_attached) {
            continue;
        }

        if (attachment.world_object_id.value != world_object_id.value) {
            continue;
        }

        if (attachment.component_type_id.value == component_type_id.value) {
            return &attachment;
        }
    }

    return nullptr;
}

WorldComponentAttachment *WorldComponentAttachmentBridge::FindFreeAttachment() {
    for (std::uint32_t index = 0U; index < snapshot_.attachment_capacity; ++index) {
        WorldComponentAttachment &attachment = attachments_[index];
        if (attachment.is_attached) {
            continue;
        }

        return &attachment;
    }

    return nullptr;
}

void WorldComponentAttachmentBridge::ClearAttachment(WorldComponentAttachment &attachment) {
    attachment.world_object_id = WorldObjectId{};
    attachment.component_type_id = WorldComponentTypeId{};
    attachment.component_slot_id = WorldComponentSlotId{};
    attachment.is_attached = false;
}

void WorldComponentAttachmentBridge::RecountActiveAttachments() {
    std::uint32_t active_attachment_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.attachment_capacity; ++index) {
        const WorldComponentAttachment &attachment = attachments_[index];
        if (!attachment.is_attached) {
            continue;
        }

        ++active_attachment_count;
    }

    snapshot_.active_attachment_count = active_attachment_count;
}
}
