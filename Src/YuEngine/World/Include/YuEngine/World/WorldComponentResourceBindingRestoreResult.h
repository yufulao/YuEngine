// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingRestoreResult.h

#pragma once

#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreState.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"

namespace yuengine::world {
struct WorldComponentResourceBindingRestoreResult final {
    WorldComponentResourceBindingRestoreStatus status =
        WorldComponentResourceBindingRestoreStatus::Success;
    WorldComponentResourceBindingStatus binding_status =
        WorldComponentResourceBindingStatus::Success;
    yuengine::resource::ResourceStatus resource_status =
        yuengine::resource::ResourceStatus::Success;
    WorldComponentResourceBindingRestoreState state{};

    /**
     * @comment Creates a successful restore result.
     * @param state Input operation state.
     * @return Explicit operation result.
     */
    static WorldComponentResourceBindingRestoreResult Success(
        const WorldComponentResourceBindingRestoreState &state) {
        return WorldComponentResourceBindingRestoreResult{
            WorldComponentResourceBindingRestoreStatus::Success,
            WorldComponentResourceBindingStatus::Success,
            yuengine::resource::ResourceStatus::Success,
            state};
    }

    /**
     * @comment Creates a failed restore result.
     * @param status Input restore status.
     * @param binding_status Input binding bridge status.
     * @param resource_status Input resource registry status.
     * @return Explicit operation result.
     */
    static WorldComponentResourceBindingRestoreResult Failure(
        WorldComponentResourceBindingRestoreStatus status,
        WorldComponentResourceBindingStatus binding_status=WorldComponentResourceBindingStatus::Success,
        yuengine::resource::ResourceStatus resource_status=yuengine::resource::ResourceStatus::Success) {
        return WorldComponentResourceBindingRestoreResult{
            status,
            binding_status,
            resource_status,
            WorldComponentResourceBindingRestoreState{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldComponentResourceBindingRestoreStatus::Success;
    }
};
}
