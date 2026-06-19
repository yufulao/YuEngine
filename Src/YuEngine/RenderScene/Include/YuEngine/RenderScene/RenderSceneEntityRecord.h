// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneEntityRecord.h

#pragma once

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Asset/AssetTextureReadyRecord.h"
#include "YuEngine/RenderCore/RenderDrawPacketRequest.h"
#include "YuEngine/RenderCore/RenderMaterialRequest.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::renderscene {
struct RenderSceneEntityRecord final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::world::WorldTransformState transform{};
    yuengine::asset::AssetHandle mesh_asset{};
    yuengine::asset::AssetHandle material_asset{};
    yuengine::asset::AssetTextureReadyRecord texture_ready{};
    yuengine::rendercore::RenderMaterialRequest material{};
    yuengine::rendercore::RenderDrawPacketRequest draw{};
    bool is_visible = true;
    bool is_active = false;
};
}
