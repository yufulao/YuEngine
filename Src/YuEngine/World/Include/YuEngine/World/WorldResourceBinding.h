// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldResourceBinding.h

#pragma once

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldResourceBinding final {
    WorldObjectId world_object_id{};
    yuengine::resource::ResourceHandle resource_handle{};
    yuengine::resource::ResourceTypeId expected_resource_type{};
    bool is_bound = false;
    bool is_acquired = false;
};
}
