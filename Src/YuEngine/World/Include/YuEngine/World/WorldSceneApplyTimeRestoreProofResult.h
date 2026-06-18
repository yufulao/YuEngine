// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneApplyTimeRestoreProofResult.h

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
     * @comment 创建成功 apply-time restore proof result。
     * @param state 输入 operation state。
     * @return 显式操作结果。
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
     * @comment 创建失败 apply-time restore proof result。
     * @param status 输入 proof status。
     * @param plan_status 输入 decoded plan status。
     * @param identity_status 输入 identity destination status。
     * @param transform_status 输入 transform destination status。
     * @param attachment_status 输入 attachment destination status。
     * @param binding_status 输入 binding destination status。
     * @param object_status 输入 object registry status。
     * @param resource_status 输入 resource registry status。
     * @return 显式操作结果。
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
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldSceneApplyTimeRestoreProofStatus::Success;
    }
};
}
