// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderMaterialResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderMaterialStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 结果 的 一个 render material 操作。
 */
struct RenderMaterialResult final {
    RenderMaterialStatus status = RenderMaterialStatus::InvalidArgument;
    std::uint32_t material_id = 0U;
    std::uint32_t program_id = 0U;
    std::uint32_t pass_id = 0U;
    std::size_t material_record_capacity = 0U;
    std::size_t current_material_record_count = 0U;
    std::size_t required_material_record_count = 0U;
    std::size_t failed_entry_index = 0U;
    std::uint32_t failed_material_id = 0U;
    std::uint32_t failed_program_id = 0U;
    std::uint32_t failed_pass_id = 0U;
    std::size_t constant_byte_count = 0U;
};
}
