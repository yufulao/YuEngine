// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h

#pragma once

#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::world {
struct WorldSceneObjectTransformRestoreTransformRecord final {
    WorldObjectId world_object_id{};
    WorldTransformState transform_state{};
};
}
