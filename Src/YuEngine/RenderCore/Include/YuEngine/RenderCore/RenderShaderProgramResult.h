// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderShaderProgramResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderShaderProgramStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 结果 的 一个 shader program 操作。
 */
struct RenderShaderProgramResult final {
    RenderShaderProgramStatus status = RenderShaderProgramStatus::InvalidArgument;
    std::uint32_t program_id = 0U;
    std::size_t required_program_record_count = 0U;
    std::size_t program_record_capacity = 0U;
    std::size_t current_program_record_count = 0U;
    std::size_t failed_entry_index = 0U;
    std::uint32_t failed_program_id = 0U;
};
}
