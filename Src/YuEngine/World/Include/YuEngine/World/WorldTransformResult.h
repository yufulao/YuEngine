// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldTransformResult.h

#pragma once

#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
struct WorldTransformResult final {
    WorldTransformStatus status = WorldTransformStatus::Success;
    WorldObjectId world_object_id{};
    WorldTransformState transform_state{};

    /**
     * @comment Creates a successful result.
     * @param world_object_id Input world object id.
     * @param transform_state Input transform state.
     * @return Explicit operation result.
     */
    static WorldTransformResult Success(WorldObjectId world_object_id,
        const WorldTransformState &transform_state) {
        return WorldTransformResult{WorldTransformStatus::Success, world_object_id, transform_state};
    }

    /**
     * @comment Creates a failed result.
     * @param status Input transform status.
     * @return Explicit operation result.
     */
    static WorldTransformResult Failure(WorldTransformStatus status) {
        return WorldTransformResult{status, WorldObjectId{}, WorldTransformState{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldTransformStatus::Success;
    }
};
}
