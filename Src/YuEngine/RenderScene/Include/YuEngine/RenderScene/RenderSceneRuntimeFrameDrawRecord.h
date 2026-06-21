// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeFrameDrawRecord.h

#pragma once

#include "YuEngine/RenderCore/RenderDrawPacketRequest.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryKind.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::renderscene {
struct RenderSceneRuntimeFrameDrawRecord final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::world::WorldTransformState transform{};
    RenderScenePrimitiveGeometryKind geometry_kind = RenderScenePrimitiveGeometryKind::Cube;
    yuengine::rendercore::RenderDrawPacketRequest draw{};
};
}
