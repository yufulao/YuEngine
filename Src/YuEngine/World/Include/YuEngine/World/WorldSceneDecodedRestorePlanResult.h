// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneDecodedRestorePlanResult.h

#pragma once

#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanState.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanStatus.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
struct WorldSceneDecodedRestorePlanResult final {
    WorldSceneDecodedRestorePlanStatus status =
        WorldSceneDecodedRestorePlanStatus::Success;
    WorldObjectIdentityStatus identity_status = WorldObjectIdentityStatus::Success;
    WorldTransformStatus transform_status = WorldTransformStatus::Success;
    WorldComponentAttachmentStatus attachment_status =
        WorldComponentAttachmentStatus::Success;
    WorldComponentResourceBindingStatus binding_status =
        WorldComponentResourceBindingStatus::Success;
    yuengine::object::ObjectStatus object_status =
        yuengine::object::ObjectStatus::Success;
    yuengine::resource::ResourceStatus resource_status =
        yuengine::resource::ResourceStatus::Success;
    WorldSceneDecodedRestorePlanState state{};

    /**
     * @comment Creates a successful decoded restore plan result.
     * @param state Input operation state.
     * @return Explicit operation result.
     */
    static WorldSceneDecodedRestorePlanResult Success(
        const WorldSceneDecodedRestorePlanState &state) {
        return WorldSceneDecodedRestorePlanResult{
            WorldSceneDecodedRestorePlanStatus::Success,
            WorldObjectIdentityStatus::Success,
            WorldTransformStatus::Success,
            WorldComponentAttachmentStatus::Success,
            WorldComponentResourceBindingStatus::Success,
            yuengine::object::ObjectStatus::Success,
            yuengine::resource::ResourceStatus::Success,
            state};
    }

    /**
     * @comment Creates a failed decoded restore plan result.
     * @param status Input plan status.
     * @param identity_status Input identity destination status.
     * @param transform_status Input transform destination status.
     * @param attachment_status Input attachment destination status.
     * @param binding_status Input binding destination status.
     * @param object_status Input object registry status.
     * @param resource_status Input resource registry status.
     * @return Explicit operation result.
     */
    static WorldSceneDecodedRestorePlanResult Failure(
        WorldSceneDecodedRestorePlanStatus status,
        WorldObjectIdentityStatus identity_status=WorldObjectIdentityStatus::Success,
        WorldTransformStatus transform_status=WorldTransformStatus::Success,
        WorldComponentAttachmentStatus attachment_status=
            WorldComponentAttachmentStatus::Success,
        WorldComponentResourceBindingStatus binding_status=
            WorldComponentResourceBindingStatus::Success,
        yuengine::object::ObjectStatus object_status=
            yuengine::object::ObjectStatus::Success,
        yuengine::resource::ResourceStatus resource_status=
            yuengine::resource::ResourceStatus::Success) {
        return WorldSceneDecodedRestorePlanResult{
            status,
            identity_status,
            transform_status,
            attachment_status,
            binding_status,
            object_status,
            resource_status,
            WorldSceneDecodedRestorePlanState{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldSceneDecodedRestorePlanStatus::Success;
    }
};
}
