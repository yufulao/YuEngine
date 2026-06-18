// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawPacketDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderDrawPacketConstants.h"

namespace yuengine::rendercore {
/**
 * @comment 描述 固定容量 RenderDrawPacket 存储.
 */
struct RenderDrawPacketDesc final {
    std::size_t draw_record_capacity = MAX_RENDER_DRAW_PACKET_RECORDS;
};
}
