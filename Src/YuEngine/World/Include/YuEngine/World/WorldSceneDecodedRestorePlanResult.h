// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneDecodedRestorePlanResult.h

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
     * @comment 创建成功 decoded restore plan result。
     * @param state 输入 operation state。
     * @return 显式操作结果。
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
     * @comment 创建失败 decoded restore plan result。
     * @param status 输入 plan status。
     * @param identity_status 输入 identity destination status。
     * @param transform_status 输入 transform destination status。
     * @param attachment_status 输入 attachment destination status。
     * @param binding_status 输入 binding destination status。
     * @param object_status 输入 object registry status。
     * @param resource_status 输入 resource registry status。
     * @return 显式操作结果。
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
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldSceneDecodedRestorePlanStatus::Success;
    }
};
}
