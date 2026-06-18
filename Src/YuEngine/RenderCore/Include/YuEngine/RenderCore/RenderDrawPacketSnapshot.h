// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawPacketSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderDrawPacketStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Snapshot of bounded RenderDrawPacket records and counters.
 */
struct RenderDrawPacketSnapshot final {
    std::size_t draw_record_capacity = 0U;
    std::size_t draw_record_count = 0U;
    std::size_t accepted_draw_count = 0U;
    std::size_t failed_validation_count = 0U;
    std::size_t duplicate_draw_id_count = 0U;
    std::size_t draw_capacity_rejected_count = 0U;
    std::uint32_t last_draw_id = 0U;
    std::uint32_t last_pass_id = 0U;
    std::uint32_t last_material_id = 0U;
    std::uint32_t last_index_count = 0U;
    RenderDrawPacketStatus last_status = RenderDrawPacketStatus::Success;
};
}
