// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentBridge.h

#pragma once

#include <array>

#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldComponentAttachmentBridgeDesc.h"
#include "YuEngine/World/WorldComponentAttachmentResult.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshot.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
class WorldComponentAttachmentBridge final {
public:
    /**
     * @comment Constructs a world component attachment bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldComponentAttachmentBridge(
        WorldComponentAttachmentBridgeDesc desc=WorldComponentAttachmentBridgeDesc{});

    /**
     * @comment Adds one caller-supplied component slot attachment.
     * @param world_object_id Input world object id.
     * @param component_type_id Input component type id.
     * @param component_slot_id Input component slot id.
     * @return Explicit operation result.
     */
    WorldComponentAttachmentResult Add(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id);
    /**
     * @comment Queries one component attachment by world object id and component type id.
     * @param world_object_id Input world object id.
     * @param component_type_id Input component type id.
     * @return Explicit operation result.
     */
    WorldComponentAttachmentResult Query(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id);
    /**
     * @comment Removes one component attachment.
     * @param world_object_id Input world object id.
     * @param component_type_id Input component type id.
     * @return Explicit operation status.
     */
    WorldComponentAttachmentStatus Remove(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id);
    /**
     * @comment Clears all component attachments in deterministic slot order.
     * @return Explicit operation status.
     */
    WorldComponentAttachmentStatus Clear();
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldComponentAttachmentSnapshot Snapshot() const;
    /**
     * @comment Copies active attachment records in deterministic slot order.
     * @param output_attachments Caller-owned output attachment buffer.
     * @param output_capacity Output attachment buffer capacity.
     * @return Total active attachment count.
     */
    std::uint32_t ExportAttachments(
        WorldComponentAttachment *output_attachments,
        std::uint32_t output_capacity) const;

private:
    WorldComponentAttachmentResult RecordFailureResult(WorldComponentAttachmentStatus status);
    WorldComponentAttachmentResult RecordDuplicateFailureResult();
    WorldComponentAttachmentStatus RecordFailure(WorldComponentAttachmentStatus status);
    void RecordSuccess();
    WorldComponentAttachmentStatus ValidateBridgeCapacity() const;
    WorldComponentAttachment *FindAttachment(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id);
    const WorldComponentAttachment *FindAttachment(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id) const;
    WorldComponentAttachment *FindFreeAttachment();
    void ClearAttachment(WorldComponentAttachment &attachment);
    void RecountActiveAttachments();

    std::array<WorldComponentAttachment, MAX_WORLD_OBJECT_COUNT> attachments_;
    WorldComponentAttachmentSnapshot snapshot_;
};
}
