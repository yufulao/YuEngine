// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeFrameEntityRequest.h

#pragma once

#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRecord.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::renderscene {
struct RenderSceneRuntimeFrameEntityRequest final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::world::WorldTransformState transform{};
    RenderScenePrimitiveGeometryRecord geometry{};
    bool is_visible = true;
    bool is_active = true;
};
}
