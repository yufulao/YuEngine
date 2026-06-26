// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeMaterialConstants.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderMaterialConstants.h"
#include "YuEngine/Rhi/RhiConstants.h"

namespace yuengine::renderscene {
constexpr std::size_t MIN_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS = 3U;
constexpr std::size_t MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS =
    yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS;
constexpr std::size_t MAX_RENDER_SCENE_RUNTIME_MATERIAL_CONSTANT_BYTES =
    yuengine::rendercore::MAX_RENDER_MATERIAL_CONSTANT_BYTES;
}
