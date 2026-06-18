// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawPacketRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

namespace yuengine::rendercore {
/**
 * @comment Describes one value-only indexed draw packet.
 */
struct RenderDrawPacketRequest final {
    std::uint32_t draw_id = 0U;
    std::uint32_t pass_id = 0U;
    std::uint32_t material_id = 0U;
    yuengine::rhi::RhiVertexBufferView vertex_buffer{};
    yuengine::rhi::RhiIndexBufferView index_buffer{};
    yuengine::rhi::RhiDrawIndexedDesc draw{};
};
}
