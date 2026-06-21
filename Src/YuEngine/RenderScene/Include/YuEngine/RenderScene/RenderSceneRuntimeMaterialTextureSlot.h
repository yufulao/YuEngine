// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeMaterialTextureSlot.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"

namespace yuengine::renderscene {
struct RenderSceneRuntimeMaterialTextureSlot final {
    std::uint32_t slot = 0U;
    yuengine::asset::AssetHandle texture_asset{};
    yuengine::rhi::RhiSampledTextureBinding sampled_texture{};
    yuengine::rhi::RhiSamplerBinding sampler{};
};
}
