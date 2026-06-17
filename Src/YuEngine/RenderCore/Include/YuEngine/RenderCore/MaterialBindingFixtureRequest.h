// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixtureRequest.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"

namespace yuengine::rendercore {
/**
 * @comment Describes one value-only material binding fixture request.
 */
struct MaterialBindingFixtureRequest final {
    std::uint32_t material_id = 0U;
    yuengine::rhi::RhiPipelineHandle pipeline{};
    yuengine::rhi::RhiSampledTextureBinding sampled_texture{};
    yuengine::rhi::RhiSamplerBinding sampler{};
    std::span<const std::uint8_t> constant_bytes{};
    std::uint32_t pass_id = 0U;
};
}
