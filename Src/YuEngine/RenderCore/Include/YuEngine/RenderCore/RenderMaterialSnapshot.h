// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderMaterialSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderMaterialStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 固定容量 render material 计数器 和 last 状态 值。
 */
struct RenderMaterialSnapshot final {
    std::size_t material_record_capacity = 0U;
    std::size_t material_record_count = 0U;
    std::size_t required_material_record_count = 0U;
    std::uint64_t accepted_material_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t duplicate_material_id_count = 0U;
    std::uint64_t material_capacity_rejected_count = 0U;
    std::uint32_t last_material_id = 0U;
    std::uint32_t last_program_id = 0U;
    std::uint32_t last_pass_id = 0U;
    std::size_t last_failed_entry_index = 0U;
    std::uint32_t last_failed_material_id = 0U;
    std::uint32_t last_failed_program_id = 0U;
    std::uint32_t last_failed_pass_id = 0U;
    std::size_t last_capacity_entry_material_record_capacity = 0U;
    std::size_t last_capacity_entry_current_material_record_count = 0U;
    std::size_t last_capacity_entry_required_material_record_count = 0U;
    std::size_t last_capacity_entry_failed_entry_index = 0U;
    std::uint32_t last_capacity_entry_material_id = 0U;
    std::uint32_t last_capacity_entry_program_id = 0U;
    std::uint32_t last_capacity_entry_pass_id = 0U;
    RenderMaterialStatus last_capacity_entry_status = RenderMaterialStatus::InvalidArgument;
    std::size_t last_constant_byte_count = 0U;
    RenderMaterialStatus last_status = RenderMaterialStatus::InvalidArgument;
};
}
