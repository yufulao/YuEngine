// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldObjectIdentityResult.h

#pragma once

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"

namespace yuengine::world {
struct WorldObjectIdentityResult final {
    WorldObjectIdentityStatus status = WorldObjectIdentityStatus::Success;
    WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};

    /**
     * @comment Creates a successful result.
     * @param world_object_id Input world object id.
     * @param object_handle Input object handle.
     * @return Explicit operation result.
     */
    static WorldObjectIdentityResult Success(WorldObjectId world_object_id,
        yuengine::object::ObjectHandle object_handle) {
        return WorldObjectIdentityResult{
            WorldObjectIdentityStatus::Success,
            world_object_id,
            object_handle};
    }

    /**
     * @comment Creates a failed result.
     * @param status Input bridge status.
     * @return Explicit operation result.
     */
    static WorldObjectIdentityResult Failure(WorldObjectIdentityStatus status) {
        return WorldObjectIdentityResult{status, WorldObjectId{}, yuengine::object::ObjectHandle{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldObjectIdentityStatus::Success;
    }
};
}
