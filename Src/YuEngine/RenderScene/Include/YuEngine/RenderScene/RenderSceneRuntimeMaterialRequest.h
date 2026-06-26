// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeMaterialRequest.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialConstants.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialTextureSlot.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"

namespace yuengine::renderscene {
struct RenderSceneRuntimeMaterialRequest final {
    yuengine::asset::AssetHandle material_asset{};
    std::uint32_t material_id = 0U;
    yuengine::rhi::RhiPipelineHandle pipeline{};
    std::span<const RenderSceneRuntimeMaterialTextureSlot> texture_slots{};
    std::span<const std::uint8_t> material_constant_bytes{};
};
}
