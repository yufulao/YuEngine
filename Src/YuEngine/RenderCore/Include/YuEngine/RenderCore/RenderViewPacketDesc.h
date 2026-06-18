// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderViewPacketDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderViewPacketConstants.h"

namespace yuengine::rendercore {
/**
 * @comment Describes bounded RenderViewPacket storage.
 */
struct RenderViewPacketDesc final {
    std::size_t view_record_capacity = MAX_RENDER_VIEW_PACKET_RECORDS;
};
}
