// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingResult.h

#pragma once

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldComponentResourceBindingResult final {
    WorldComponentResourceBindingStatus status = WorldComponentResourceBindingStatus::Success;
    yuengine::resource::ResourceStatus resource_status = yuengine::resource::ResourceStatus::Success;
    WorldObjectId world_object_id{};
    WorldComponentTypeId component_type_id{};
    WorldComponentSlotId component_slot_id{};
    yuengine::resource::ResourceHandle resource_handle{};
    yuengine::resource::ResourceTypeId expected_resource_type{};

    /**
     * @comment Creates a successful result.
     * @param world_object_id Input world object id.
     * @param component_type_id Input component type id.
     * @param component_slot_id Input component slot id.
     * @param resource_handle Input resource handle.
     * @param expected_resource_type Input expected resource type.
     * @return Explicit operation result.
     */
    static WorldComponentResourceBindingResult Success(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id,
        yuengine::resource::ResourceHandle resource_handle,
        yuengine::resource::ResourceTypeId expected_resource_type) {
        return WorldComponentResourceBindingResult{
            WorldComponentResourceBindingStatus::Success,
            yuengine::resource::ResourceStatus::Success,
            world_object_id,
            component_type_id,
            component_slot_id,
            resource_handle,
            expected_resource_type};
    }

    /**
     * @comment Creates a failed result.
     * @param status Input bridge status.
     * @param resource_status Input resource status.
     * @return Explicit operation result.
     */
    static WorldComponentResourceBindingResult Failure(
        WorldComponentResourceBindingStatus status,
        yuengine::resource::ResourceStatus resource_status=yuengine::resource::ResourceStatus::Success) {
        return WorldComponentResourceBindingResult{
            status,
            resource_status,
            WorldObjectId{},
            WorldComponentTypeId{},
            WorldComponentSlotId{},
            yuengine::resource::ResourceHandle{},
            yuengine::resource::ResourceTypeId{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldComponentResourceBindingStatus::Success;
    }
};
}
