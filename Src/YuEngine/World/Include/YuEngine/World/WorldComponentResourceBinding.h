// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBinding.h

#pragma once

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldComponentResourceBinding final {
    WorldObjectId world_object_id{};
    WorldComponentTypeId component_type_id{};
    WorldComponentSlotId component_slot_id{};
    yuengine::resource::ResourceHandle resource_handle{};
    yuengine::resource::ResourceTypeId expected_resource_type{};
    bool is_bound = false;
    bool is_acquired = false;
};
}
