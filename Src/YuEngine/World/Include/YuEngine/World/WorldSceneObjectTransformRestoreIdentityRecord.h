// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h

#pragma once

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldSceneObjectTransformRestoreIdentityRecord final {
    WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};
};
}
