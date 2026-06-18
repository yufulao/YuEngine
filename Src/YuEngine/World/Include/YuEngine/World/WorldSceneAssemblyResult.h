// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyResult.h

#pragma once

#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldSceneAssemblyState.h"
#include "YuEngine/World/WorldSceneAssemblyStatus.h"

namespace yuengine::world {
struct WorldSceneAssemblyResult final {
    WorldSceneAssemblyStatus status = WorldSceneAssemblyStatus::Success;
    WorldComponentAttachmentStatus attachment_status =
        WorldComponentAttachmentStatus::Success;
    WorldComponentResourceBindingStatus binding_status =
        WorldComponentResourceBindingStatus::Success;
    WorldComponentResourceBindingRestoreStatus binding_restore_status =
        WorldComponentResourceBindingRestoreStatus::Success;
    yuengine::resource::ResourceStatus resource_status =
        yuengine::resource::ResourceStatus::Success;
    WorldSceneAssemblyState state{};

    /**
     * @comment 创建成功 scene assembly result。
     * @param state 输入 operation state。
     * @return 显式操作结果。
     */
    static WorldSceneAssemblyResult Success(const WorldSceneAssemblyState &state) {
        return WorldSceneAssemblyResult{
            WorldSceneAssemblyStatus::Success,
            WorldComponentAttachmentStatus::Success,
            WorldComponentResourceBindingStatus::Success,
            WorldComponentResourceBindingRestoreStatus::Success,
            yuengine::resource::ResourceStatus::Success,
            state};
    }

    /**
     * @comment 创建失败 scene assembly result。
     * @param status 输入 scene assembly status。
     * @param attachment_status 输入 component attachment status。
     * @param binding_status 输入 component-resource binding status。
     * @param binding_restore_status 输入 binding restore status。
     * @param resource_status 输入 resource status。
     * @return 显式操作结果。
     */
    static WorldSceneAssemblyResult Failure(
        WorldSceneAssemblyStatus status,
        WorldComponentAttachmentStatus attachment_status=
            WorldComponentAttachmentStatus::Success,
        WorldComponentResourceBindingStatus binding_status=
            WorldComponentResourceBindingStatus::Success,
        WorldComponentResourceBindingRestoreStatus binding_restore_status=
            WorldComponentResourceBindingRestoreStatus::Success,
        yuengine::resource::ResourceStatus resource_status=
            yuengine::resource::ResourceStatus::Success) {
        return WorldSceneAssemblyResult{
            status,
            attachment_status,
            binding_status,
            binding_restore_status,
            resource_status,
            WorldSceneAssemblyState{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldSceneAssemblyStatus::Success;
    }
};
}
