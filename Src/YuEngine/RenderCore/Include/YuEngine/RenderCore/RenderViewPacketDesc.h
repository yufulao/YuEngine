// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderViewPacketDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderViewPacketConstants.h"

namespace yuengine::rendercore {
/**
 * @comment 描述 固定容量 RenderViewPacket 存储.
 */
struct RenderViewPacketDesc final {
    std::size_t view_record_capacity = MAX_RENDER_VIEW_PACKET_RECORDS;
};
}
