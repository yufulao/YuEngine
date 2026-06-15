// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldResourceBindingResult.h

#pragma once

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldResourceBindingStatus.h"

namespace yuengine::world {
struct WorldResourceBindingResult final {
    WorldResourceBindingStatus status = WorldResourceBindingStatus::Success;
    yuengine::resource::ResourceStatus resource_status = yuengine::resource::ResourceStatus::Success;
    WorldObjectId world_object_id{};
    yuengine::resource::ResourceHandle resource_handle{};
    yuengine::resource::ResourceTypeId expected_resource_type{};

    /**
     * @comment Creates a successful result.
     * @param world_object_id Input world object id.
     * @param resource_handle Input resource handle.
     * @param expected_resource_type Input expected resource type.
     * @return Explicit operation result.
     */
    static WorldResourceBindingResult Success(WorldObjectId world_object_id,
        yuengine::resource::ResourceHandle resource_handle,
        yuengine::resource::ResourceTypeId expected_resource_type) {
        return WorldResourceBindingResult{
            WorldResourceBindingStatus::Success,
            yuengine::resource::ResourceStatus::Success,
            world_object_id,
            resource_handle,
            expected_resource_type};
    }

    /**
     * @comment Creates a failed result.
     * @param status Input bridge status.
     * @param resource_status Input resource status.
     * @return Explicit operation result.
     */
    static WorldResourceBindingResult Failure(WorldResourceBindingStatus status,
        yuengine::resource::ResourceStatus resource_status=yuengine::resource::ResourceStatus::Success) {
        return WorldResourceBindingResult{
            status,
            resource_status,
            WorldObjectId{},
            yuengine::resource::ResourceHandle{},
            yuengine::resource::ResourceTypeId{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldResourceBindingStatus::Success;
    }
};
}
