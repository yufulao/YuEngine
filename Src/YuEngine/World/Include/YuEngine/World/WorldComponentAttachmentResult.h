// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentResult.h

#pragma once

#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldComponentAttachmentResult final {
    WorldComponentAttachmentStatus status = WorldComponentAttachmentStatus::Success;
    WorldObjectId world_object_id{};
    WorldComponentTypeId component_type_id{};
    WorldComponentSlotId component_slot_id{};

    /**
     * @comment Creates a successful result.
     * @param world_object_id Input world object id.
     * @param component_type_id Input component type id.
     * @param component_slot_id Input component slot id.
     * @return Explicit operation result.
     */
    static WorldComponentAttachmentResult Success(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id) {
        return WorldComponentAttachmentResult{
            WorldComponentAttachmentStatus::Success,
            world_object_id,
            component_type_id,
            component_slot_id};
    }

    /**
     * @comment Creates a failed result.
     * @param status Input attachment status.
     * @return Explicit operation result.
     */
    static WorldComponentAttachmentResult Failure(WorldComponentAttachmentStatus status) {
        return WorldComponentAttachmentResult{
            status,
            WorldObjectId{},
            WorldComponentTypeId{},
            WorldComponentSlotId{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldComponentAttachmentStatus::Success;
    }
};
}
