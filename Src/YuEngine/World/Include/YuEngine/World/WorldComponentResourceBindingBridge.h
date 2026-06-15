// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingBridge.h

#pragma once

#include <array>

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldComponentResourceBinding.h"
#include "YuEngine/World/WorldComponentResourceBindingBridgeDesc.h"
#include "YuEngine/World/WorldComponentResourceBindingResult.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshot.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::world {
class WorldComponentAttachmentBridge;

class WorldComponentResourceBindingBridge final {
public:
    /**
     * @comment Constructs a world component resource binding bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldComponentResourceBindingBridge(
        WorldComponentResourceBindingBridgeDesc desc=WorldComponentResourceBindingBridgeDesc{});

    /**
     * @comment Binds one existing component attachment tuple to one resource handle.
     * @param attachment_source Caller-owned component attachment source.
     * @param resource_registry Caller-owned resource registry.
     * @param world_object_id Input world object id.
     * @param component_type_id Input component type id.
     * @param component_slot_id Input component slot id.
     * @param resource_handle Input resource handle.
     * @param expected_resource_type Input expected resource type.
     * @return Explicit operation result.
     */
    WorldComponentResourceBindingResult Bind(
        const WorldComponentAttachmentBridge *attachment_source,
        yuengine::resource::ResourceRegistry *resource_registry,
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id,
        yuengine::resource::ResourceHandle resource_handle,
        yuengine::resource::ResourceTypeId expected_resource_type);
    /**
     * @comment Queries one component resource binding by component attachment tuple.
     * @param world_object_id Input world object id.
     * @param component_type_id Input component type id.
     * @param component_slot_id Input component slot id.
     * @return Explicit operation result.
     */
    WorldComponentResourceBindingResult Query(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id);
    /**
     * @comment Removes one component resource binding and releases its resource handle.
     * @param resource_registry Caller-owned resource registry.
     * @param world_object_id Input world object id.
     * @param component_type_id Input component type id.
     * @param component_slot_id Input component slot id.
     * @return Explicit operation status.
     */
    WorldComponentResourceBindingStatus Remove(
        yuengine::resource::ResourceRegistry *resource_registry,
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id);
    /**
     * @comment Releases and clears all active component resource bindings in deterministic slot order.
     * @param resource_registry Caller-owned resource registry.
     * @return Explicit operation status.
     */
    WorldComponentResourceBindingStatus Clear(yuengine::resource::ResourceRegistry *resource_registry);
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldComponentResourceBindingSnapshot Snapshot() const;

private:
    WorldComponentResourceBindingResult RecordFailureResult(WorldComponentResourceBindingStatus status);
    WorldComponentResourceBindingResult RecordFailureResult(
        WorldComponentResourceBindingStatus status,
        yuengine::resource::ResourceStatus resource_status);
    WorldComponentResourceBindingStatus RecordFailure(WorldComponentResourceBindingStatus status);
    WorldComponentResourceBindingStatus RecordFailure(
        WorldComponentResourceBindingStatus status,
        yuengine::resource::ResourceStatus resource_status);
    void RecordSuccess(yuengine::resource::ResourceStatus resource_status);
    WorldComponentResourceBindingStatus ValidateBridgeCapacity() const;
    WorldComponentResourceBindingStatus ValidateAttachmentTuple(
        const WorldComponentAttachmentBridge *attachment_source,
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id) const;
    WorldComponentResourceBindingStatus MapAcquireStatus(yuengine::resource::ResourceStatus resource_status) const;
    WorldComponentResourceBinding *FindBinding(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id);
    const WorldComponentResourceBinding *FindBinding(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id) const;
    WorldComponentResourceBinding *FindFreeBinding();
    void ClearBinding(WorldComponentResourceBinding &binding);
    void RecountActiveBindings();

    std::array<WorldComponentResourceBinding, MAX_WORLD_OBJECT_COUNT> bindings_;
    WorldComponentResourceBindingSnapshot snapshot_;
};
}
