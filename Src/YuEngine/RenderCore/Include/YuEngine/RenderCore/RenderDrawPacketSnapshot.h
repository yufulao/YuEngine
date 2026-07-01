// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawPacketSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderDrawPacketStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 固定容量 RenderDrawPacket 记录 和 计数器 的快照。
 */
struct RenderDrawPacketSnapshot final {
    std::size_t draw_record_capacity = 0U;
    std::size_t draw_record_count = 0U;
    std::size_t required_draw_record_count = 0U;
    std::size_t accepted_draw_count = 0U;
    std::size_t failed_validation_count = 0U;
    std::size_t duplicate_draw_id_count = 0U;
    std::size_t draw_capacity_rejected_count = 0U;
    std::size_t last_capacity_entry_draw_record_capacity = 0U;
    std::size_t last_capacity_entry_current_draw_record_count = 0U;
    std::size_t last_capacity_entry_required_draw_record_count = 0U;
    std::size_t last_capacity_entry_failed_entry_index = 0U;
    std::uint32_t last_capacity_entry_draw_id = 0U;
    std::uint32_t last_capacity_entry_pass_id = 0U;
    std::uint32_t last_capacity_entry_material_id = 0U;
    std::uint32_t last_capacity_entry_index_count = 0U;
    RenderDrawPacketStatus last_capacity_entry_status = RenderDrawPacketStatus::Success;
    std::size_t last_failed_entry_index = 0U;
    std::uint32_t last_failed_draw_id = 0U;
    std::uint32_t last_failed_pass_id = 0U;
    std::uint32_t last_failed_material_id = 0U;
    std::uint32_t last_draw_id = 0U;
    std::uint32_t last_pass_id = 0U;
    std::uint32_t last_material_id = 0U;
    std::uint32_t last_index_count = 0U;
    RenderDrawPacketStatus last_status = RenderDrawPacketStatus::Success;
};
}
