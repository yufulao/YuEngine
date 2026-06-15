// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneApplyTimeRestoreProofResult.h

#pragma once

#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofState.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofStatus.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanStatus.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
struct WorldSceneApplyTimeRestoreProofResult final {
    WorldSceneApplyTimeRestoreProofStatus status =
        WorldSceneApplyTimeRestoreProofStatus::Success;
    WorldSceneDecodedRestorePlanStatus plan_status =
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
    WorldSceneApplyTimeRestoreProofState state{};

    /**
     * @comment Creates a successful apply-time restore proof result.
     * @param state Input operation state.
     * @return Explicit operation result.
     */
    static WorldSceneApplyTimeRestoreProofResult Success(
        const WorldSceneApplyTimeRestoreProofState &state) {
        return WorldSceneApplyTimeRestoreProofResult{
            WorldSceneApplyTimeRestoreProofStatus::Success,
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
     * @comment Creates a failed apply-time restore proof result.
     * @param status Input proof status.
     * @param plan_status Input decoded plan status.
     * @param identity_status Input identity destination status.
     * @param transform_status Input transform destination status.
     * @param attachment_status Input attachment destination status.
     * @param binding_status Input binding destination status.
     * @param object_status Input object registry status.
     * @param resource_status Input resource registry status.
     * @return Explicit operation result.
     */
    static WorldSceneApplyTimeRestoreProofResult Failure(
        WorldSceneApplyTimeRestoreProofStatus status,
        WorldSceneDecodedRestorePlanStatus plan_status=
            WorldSceneDecodedRestorePlanStatus::Success,
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
        return WorldSceneApplyTimeRestoreProofResult{
            status,
            plan_status,
            identity_status,
            transform_status,
            attachment_status,
            binding_status,
            object_status,
            resource_status,
            WorldSceneApplyTimeRestoreProofState{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldSceneApplyTimeRestoreProofStatus::Success;
    }
};
}
