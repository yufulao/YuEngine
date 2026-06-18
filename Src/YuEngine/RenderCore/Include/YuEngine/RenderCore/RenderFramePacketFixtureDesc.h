// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFramePacketFixtureDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderFramePacketFixtureConstants.h"

namespace yuengine::rendercore {
/**
 * @comment 配置 固定容量 存储 用于 一个 RenderCore frame packet fixture.
 */
struct RenderFramePacketFixtureDesc final {
    std::size_t frame_packet_record_capacity = MAX_RENDER_FRAME_PACKET_FIXTURE_RECORDS;
};
}
