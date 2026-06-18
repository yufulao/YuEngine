// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawPacketResult.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderDrawPacketStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Result for one RenderDrawPacket build request.
 */
struct RenderDrawPacketResult final {
    RenderDrawPacketStatus status = RenderDrawPacketStatus::Success;
    std::uint32_t draw_id = 0U;
    std::uint32_t pass_id = 0U;
    std::uint32_t material_id = 0U;
    std::uint32_t index_count = 0U;
};
}
