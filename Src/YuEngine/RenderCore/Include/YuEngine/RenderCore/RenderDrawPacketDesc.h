// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawPacketDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderDrawPacketConstants.h"

namespace yuengine::rendercore {
/**
 * @comment Describes bounded RenderDrawPacket storage.
 */
struct RenderDrawPacketDesc final {
    std::size_t draw_record_capacity = MAX_RENDER_DRAW_PACKET_RECORDS;
};
}
