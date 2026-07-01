// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderShaderProgramSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderShaderProgramStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 固定容量 shader program 计数器 和 last 状态 值。
 */
struct RenderShaderProgramSnapshot final {
    std::size_t program_record_capacity = 0U;
    std::size_t program_record_count = 0U;
    std::uint64_t accepted_program_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t duplicate_program_id_count = 0U;
    std::uint64_t program_capacity_rejected_count = 0U;
    std::uint32_t last_program_id = 0U;
    std::size_t last_required_program_record_count = 0U;
    std::size_t last_failed_program_record_capacity = 0U;
    std::size_t last_failed_program_record_count = 0U;
    std::size_t last_failed_entry_index = 0U;
    std::uint32_t last_failed_program_id = 0U;
    RenderShaderProgramStatus last_status = RenderShaderProgramStatus::InvalidArgument;
};
}
