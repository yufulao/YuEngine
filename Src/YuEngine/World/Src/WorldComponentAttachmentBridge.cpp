// Module: YuEngine World
// File: Src/YuEngine/World/Src/WorldComponentAttachmentBridge.cpp

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

WorldComponentAttachmentResult WorldComponentAttachmentBridge::RecordFailureResult(
    WorldComponentAttachmentStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return WorldComponentAttachmentResult::Failure(status);
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
