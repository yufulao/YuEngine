// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyResult.h

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
     * @comment Creates a successful scene assembly result.
     * @param state Input operation state.
     * @return Explicit operation result.
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
     * @comment Creates a failed scene assembly result.
     * @param status Input scene assembly status.
     * @param attachment_status Input component attachment status.
     * @param binding_status Input component-resource binding status.
     * @param binding_restore_status Input binding restore status.
     * @param resource_status Input resource status.
     * @return Explicit operation result.
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
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldSceneAssemblyStatus::Success;
    }
};
}
