// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformRestoreResult.h

#pragma once

#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreState.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreStatus.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
struct WorldSceneObjectTransformRestoreResult final {
    WorldSceneObjectTransformRestoreStatus status =
        WorldSceneObjectTransformRestoreStatus::Success;
    WorldObjectIdentityStatus identity_status = WorldObjectIdentityStatus::Success;
    WorldTransformStatus transform_status = WorldTransformStatus::Success;
    yuengine::object::ObjectStatus object_status = yuengine::object::ObjectStatus::Success;
    WorldSceneObjectTransformRestoreState state{};

    /**
     * @comment Creates a successful object-transform restore result.
     * @param state Input operation state.
     * @return Explicit operation result.
     */
    static WorldSceneObjectTransformRestoreResult Success(
        const WorldSceneObjectTransformRestoreState &state) {
        return WorldSceneObjectTransformRestoreResult{
            WorldSceneObjectTransformRestoreStatus::Success,
            WorldObjectIdentityStatus::Success,
            WorldTransformStatus::Success,
            yuengine::object::ObjectStatus::Success,
            state};
    }

    /**
     * @comment Creates a failed object-transform restore result.
     * @param status Input restore status.
     * @param identity_status Input identity bridge status.
     * @param transform_status Input transform bridge status.
     * @param object_status Input object registry status.
     * @return Explicit operation result.
     */
    static WorldSceneObjectTransformRestoreResult Failure(
        WorldSceneObjectTransformRestoreStatus status,
        WorldObjectIdentityStatus identity_status=WorldObjectIdentityStatus::Success,
        WorldTransformStatus transform_status=WorldTransformStatus::Success,
        yuengine::object::ObjectStatus object_status=
            yuengine::object::ObjectStatus::Success) {
        return WorldSceneObjectTransformRestoreResult{
            status,
            identity_status,
            transform_status,
            object_status,
            WorldSceneObjectTransformRestoreState{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldSceneObjectTransformRestoreStatus::Success;
    }
};
}
