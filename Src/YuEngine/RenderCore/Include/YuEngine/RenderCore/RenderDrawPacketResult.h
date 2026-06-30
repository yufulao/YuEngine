// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawPacketResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderDrawPacketStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 一个 RenderDrawPacket build 请求 的结果。
 */
struct RenderDrawPacketResult final {
    RenderDrawPacketStatus status = RenderDrawPacketStatus::Success;
    std::uint32_t draw_id = 0U;
    std::uint32_t pass_id = 0U;
    std::uint32_t material_id = 0U;
    std::uint32_t index_count = 0U;
    std::size_t required_draw_record_count = 0U;
};
}
