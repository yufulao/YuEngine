// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderViewPacketSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderDrawPacketStatus.h"
#include "YuEngine/RenderCore/RenderMaterialStatus.h"
#include "YuEngine/RenderCore/RenderViewPacketStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Snapshot of bounded RenderViewPacket records and counters.
 */
struct RenderViewPacketSnapshot final {
    std::size_t view_record_capacity = 0U;
    std::size_t view_record_count = 0U;
    std::size_t accepted_view_count = 0U;
    std::size_t failed_validation_count = 0U;
    std::size_t material_failure_count = 0U;
    std::size_t draw_failure_count = 0U;
    std::size_t duplicate_view_id_count = 0U;
    std::size_t view_capacity_rejected_count = 0U;
    std::uint32_t last_view_id = 0U;
    std::uint32_t last_frame_id = 0U;
    std::uint32_t last_pass_id = 0U;
    std::uint32_t last_material_id = 0U;
    std::uint32_t last_draw_id = 0U;
    std::uint32_t last_index_count = 0U;
    std::size_t last_constant_byte_count = 0U;
    std::size_t last_capture_byte_budget = 0U;
    RenderViewPacketStatus last_status = RenderViewPacketStatus::Success;
    RenderMaterialStatus last_material_status = RenderMaterialStatus::Success;
    RenderDrawPacketStatus last_draw_status = RenderDrawPacketStatus::Success;
};
}
