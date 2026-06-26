// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixtureRequest.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Rhi/RhiConstantBufferBinding.h"
#include "YuEngine/Rhi/RhiBlendStateDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"

namespace yuengine::rendercore {
/**
 * @comment 描述 一个 值-仅 material 绑定 fixture 请求.
 */
struct MaterialBindingFixtureRequest final {
    std::uint32_t material_id = 0U;
    yuengine::rhi::RhiPipelineHandle pipeline{};
    yuengine::rhi::RhiBlendStateDesc blend_state{};
    yuengine::rhi::RhiSampledTextureBinding sampled_texture{};
    yuengine::rhi::RhiSamplerBinding sampler{};
    std::span<const yuengine::rhi::RhiSampledTextureBinding> sampled_textures{};
    std::span<const yuengine::rhi::RhiSamplerBinding> samplers{};
    std::span<const yuengine::rhi::RhiConstantBufferBinding> constant_buffers{};
    std::span<const std::uint8_t> constant_bytes{};
    std::uint32_t pass_id = 0U;
};
}
