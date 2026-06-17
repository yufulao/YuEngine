// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePassRequest.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

namespace yuengine::rendercore {
/**
 * @comment Describes one synthetic RenderCore fixture pass request.
 */
struct RenderFixturePassRequest final {
    yuengine::rhi::IRhiDevice *rhi_device = nullptr;
    yuengine::rhi::RhiTextureHandle target{};
    yuengine::rhi::RhiPipelineHandle pipeline{};
    yuengine::rhi::RhiVertexBufferView vertex_buffer{};
    yuengine::rhi::RhiIndexBufferView index_buffer{};
    yuengine::rhi::RhiSampledTextureBinding sampled_texture{};
    yuengine::rhi::RhiSamplerBinding sampler{};
    yuengine::rhi::RhiDrawIndexedDesc draw{};
    yuengine::rhi::RhiColor clear_color{};
    std::span<std::uint8_t> capture_output{};
    std::size_t capture_byte_budget = 0U;
    std::uint32_t pass_id = 0U;
};
}
