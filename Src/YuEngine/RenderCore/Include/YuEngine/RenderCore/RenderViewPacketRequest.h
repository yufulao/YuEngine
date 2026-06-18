// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderViewPacketRequest.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderCore/RenderDrawPacketRequest.h"
#include "YuEngine/RenderCore/RenderMaterialRequest.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rendercore {
/**
 * @comment Describes one value-only view packet assembled from material and draw inputs.
 */
struct RenderViewPacketRequest final {
    std::uint32_t view_id = 0U;
    std::uint32_t frame_id = 0U;
    yuengine::rhi::RhiTextureHandle target{};
    yuengine::rhi::RhiColor clear_color{};
    std::span<std::uint8_t> capture_output{};
    std::size_t capture_byte_budget = 0U;
    RenderMaterialRequest material{};
    RenderDrawPacketRequest draw{};
};
}
