// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldTransformBridge.h

#pragma once

#include <array>

#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldTransformBinding.h"
#include "YuEngine/World/WorldTransformBridgeDesc.h"
#include "YuEngine/World/WorldTransformResult.h"
#include "YuEngine/World/WorldTransformSnapshot.h"
#include "YuEngine/World/WorldTransformState.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
class WorldTransformBridge final {
public:
    /**
     * @comment Constructs a transform data bridge for one WorldInstance.
     * @param world World instance queried by the bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldTransformBridge(WorldInstance &world,
        WorldTransformBridgeDesc desc=WorldTransformBridgeDesc{});

    /**
     * @comment Registers transform state for an existing world object.
     * @param world_object_id Input world object id.
     * @param transform_state Input transform state.
     * @return Explicit operation result.
     */
    WorldTransformResult Register(WorldObjectId world_object_id,
        const WorldTransformState &transform_state);
    /**
     * @comment Updates transform state for an existing transform record.
     * @param world_object_id Input world object id.
     * @param transform_state Input transform state.
     * @return Explicit operation status.
     */
    WorldTransformStatus Set(WorldObjectId world_object_id,
        const WorldTransformState &transform_state);
    /**
     * @comment Queries transform state for an existing transform record.
     * @param world_object_id Input world object id.
     * @return Explicit operation result.
     */
    WorldTransformResult Query(WorldObjectId world_object_id);
    /**
     * @comment Removes one transform record.
     * @param world_object_id Input world object id.
     * @return Explicit operation status.
     */
    WorldTransformStatus Remove(WorldObjectId world_object_id);
    /**
     * @comment Removes all transform records.
     * @return Explicit operation status.
     */
    WorldTransformStatus Clear();
    /**
     * @comment Returns a snapshot of the current transform bridge state.
     * @return Snapshot value.
     */
    WorldTransformSnapshot Snapshot() const;

private:
    WorldTransformStatus RecordFailure(WorldTransformStatus status);
    void RecordSuccess();
    WorldTransformStatus ValidateBridgeCapacity() const;
    WorldTransformBinding *FindBindingByWorldObjectId(WorldObjectId world_object_id);
    const WorldTransformBinding *FindBindingByWorldObjectId(WorldObjectId world_object_id) const;
    WorldTransformBinding *FindFreeBinding();
    void ClearBinding(WorldTransformBinding &binding);
    void RecountRecords();

    WorldInstance &world_;
    std::array<WorldTransformBinding, MAX_WORLD_OBJECT_COUNT> bindings_;
    WorldTransformSnapshot snapshot_;
};
}
