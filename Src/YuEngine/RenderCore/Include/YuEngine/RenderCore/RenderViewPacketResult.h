// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderViewPacketResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderDrawPacketStatus.h"
#include "YuEngine/RenderCore/RenderMaterialStatus.h"
#include "YuEngine/RenderCore/RenderViewPacketStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Result for one RenderViewPacket build request.
 */
struct RenderViewPacketResult final {
    RenderViewPacketStatus status = RenderViewPacketStatus::Success;
    RenderMaterialStatus material_status = RenderMaterialStatus::Success;
    RenderDrawPacketStatus draw_status = RenderDrawPacketStatus::Success;
    std::uint32_t view_id = 0U;
    std::uint32_t frame_id = 0U;
    std::uint32_t pass_id = 0U;
    std::uint32_t material_id = 0U;
    std::uint32_t draw_id = 0U;
    std::uint32_t index_count = 0U;
    std::size_t constant_byte_count = 0U;
    std::size_t capture_byte_budget = 0U;
};
}
