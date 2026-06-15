// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldResourceBindingBridge.h

#pragma once

#include <array>

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldResourceBinding.h"
#include "YuEngine/World/WorldResourceBindingBridgeDesc.h"
#include "YuEngine/World/WorldResourceBindingResult.h"
#include "YuEngine/World/WorldResourceBindingSnapshot.h"
#include "YuEngine/World/WorldResourceBindingStatus.h"

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::world {
class WorldResourceBindingBridge final {
public:
    /**
     * @comment Constructs a world resource binding bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldResourceBindingBridge(
        WorldResourceBindingBridgeDesc desc=WorldResourceBindingBridgeDesc{});

    /**
     * @comment Binds one caller-supplied world object id to one resource handle and acquires the handle.
     * @param resource_registry Caller-owned resource registry.
     * @param world_object_id Input world object id.
     * @param resource_handle Input resource handle.
     * @param expected_resource_type Input expected resource type.
     * @return Explicit operation result.
     */
    WorldResourceBindingResult Bind(
        yuengine::resource::ResourceRegistry *resource_registry,
        WorldObjectId world_object_id,
        yuengine::resource::ResourceHandle resource_handle,
        yuengine::resource::ResourceTypeId expected_resource_type);
    /**
     * @comment Queries one binding by world object id.
     * @param world_object_id Input world object id.
     * @return Explicit operation result.
     */
    WorldResourceBindingResult Query(WorldObjectId world_object_id);
    /**
     * @comment Removes one binding and releases its acquired resource handle.
     * @param resource_registry Caller-owned resource registry.
     * @param world_object_id Input world object id.
     * @return Explicit operation status.
     */
    WorldResourceBindingStatus Remove(
        yuengine::resource::ResourceRegistry *resource_registry,
        WorldObjectId world_object_id);
    /**
     * @comment Releases and clears all active bindings in deterministic slot order.
     * @param resource_registry Caller-owned resource registry.
     * @return Explicit operation status.
     */
    WorldResourceBindingStatus Clear(yuengine::resource::ResourceRegistry *resource_registry);
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldResourceBindingSnapshot Snapshot() const;

private:
    WorldResourceBindingResult RecordFailureResult(WorldResourceBindingStatus status);
    WorldResourceBindingResult RecordFailureResult(WorldResourceBindingStatus status,
        yuengine::resource::ResourceStatus resource_status);
    WorldResourceBindingStatus RecordFailure(WorldResourceBindingStatus status);
    WorldResourceBindingStatus RecordFailure(WorldResourceBindingStatus status,
        yuengine::resource::ResourceStatus resource_status);
    void RecordSuccess(yuengine::resource::ResourceStatus resource_status);
    WorldResourceBindingStatus ValidateBridgeCapacity() const;
    WorldResourceBindingStatus MapAcquireStatus(yuengine::resource::ResourceStatus resource_status) const;
    WorldResourceBinding *FindBindingByWorldObjectId(WorldObjectId world_object_id);
    const WorldResourceBinding *FindBindingByWorldObjectId(WorldObjectId world_object_id) const;
    WorldResourceBinding *FindFreeBinding();
    void ClearBinding(WorldResourceBinding &binding);
    void RecountActiveBindings();

    std::array<WorldResourceBinding, MAX_WORLD_OBJECT_COUNT> bindings_;
    WorldResourceBindingSnapshot snapshot_;
};
}
