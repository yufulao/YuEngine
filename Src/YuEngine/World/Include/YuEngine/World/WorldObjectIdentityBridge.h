// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldObjectIdentityBridge.h

#pragma once

#include <array>

#include "YuEngine/Object/ObjectRegistry.h"
#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldObjectIdentityBinding.h"
#include "YuEngine/World/WorldObjectIdentityBridgeDesc.h"
#include "YuEngine/World/WorldObjectIdentityResult.h"
#include "YuEngine/World/WorldObjectIdentitySnapshot.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"

namespace yuengine::world {
class WorldObjectIdentityBridge final {
public:
    /**
     * @comment Constructs a bridge between one WorldInstance and one ObjectRegistry.
     * @param world World instance queried by the bridge.
     * @param object_registry Object registry used for handle lifetime operations.
     * @param desc Input bridge descriptor.
     */
    WorldObjectIdentityBridge(WorldInstance &world,
        yuengine::object::ObjectRegistry &object_registry,
        WorldObjectIdentityBridgeDesc desc=WorldObjectIdentityBridgeDesc{});

    /**
     * @comment Binds one world object id to one object handle and acquires the handle.
     * @param world_object_id Input world object id.
     * @param object_handle Input object handle.
     * @return Explicit operation result.
     */
    WorldObjectIdentityResult Bind(WorldObjectId world_object_id,
        yuengine::object::ObjectHandle object_handle);
    /**
     * @comment Validates an existing binding through ObjectRegistry.
     * @param world_object_id Input world object id.
     * @return Explicit operation status.
     */
    WorldObjectIdentityStatus Validate(WorldObjectId world_object_id);
    /**
     * @comment Removes one binding and releases its acquired object handle.
     * @param world_object_id Input world object id.
     * @return Explicit operation status.
     */
    WorldObjectIdentityStatus Remove(WorldObjectId world_object_id);
    /**
     * @comment Releases all active bindings.
     * @return Explicit operation status.
     */
    WorldObjectIdentityStatus Clear();
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldObjectIdentitySnapshot Snapshot() const;

private:
    WorldObjectIdentityStatus RecordFailure(WorldObjectIdentityStatus status);
    WorldObjectIdentityStatus RecordFailure(WorldObjectIdentityStatus status,
        yuengine::object::ObjectStatus object_status);
    void RecordSuccess(yuengine::object::ObjectStatus object_status);
    WorldObjectIdentityStatus ValidateBridgeCapacity() const;
    WorldObjectIdentityStatus MapObjectStatus(yuengine::object::ObjectStatus object_status) const;
    WorldObjectIdentityBinding *FindBindingByWorldObjectId(WorldObjectId world_object_id);
    const WorldObjectIdentityBinding *FindBindingByWorldObjectId(WorldObjectId world_object_id) const;
    WorldObjectIdentityBinding *FindBindingByObjectHandle(yuengine::object::ObjectHandle object_handle);
    WorldObjectIdentityBinding *FindFreeBinding();
    void ClearBinding(WorldObjectIdentityBinding &binding);
    void RecountActiveBindings();

    WorldInstance &world_;
    yuengine::object::ObjectRegistry &object_registry_;
    std::array<WorldObjectIdentityBinding, MAX_WORLD_OBJECT_COUNT> bindings_;
    WorldObjectIdentitySnapshot snapshot_;
};
}
