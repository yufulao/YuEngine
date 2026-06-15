// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldObjectIdentityBinding.h

#pragma once

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldObjectIdentityBinding final {
    WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};
    bool is_bound = false;
    bool is_acquired = false;
};
}
