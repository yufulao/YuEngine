// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixtureSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/MaterialBindingFixtureStatus.h"
#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 固定容量 material 绑定 fixture 计数器 和 last 状态 值。
 */
struct MaterialBindingFixtureSnapshot final {
    std::size_t binding_record_capacity = 0U;
    std::size_t binding_record_count = 0U;
    std::uint64_t accepted_binding_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t duplicate_material_id_count = 0U;
    std::uint64_t binding_capacity_rejected_count = 0U;
    std::uint64_t executed_pass_count = 0U;
    std::uint64_t completed_pass_count = 0U;
    std::uint64_t render_pass_failure_count = 0U;
    std::uint32_t last_material_id = 0U;
    std::uint32_t last_pass_id = 0U;
    std::size_t last_constant_byte_count = 0U;
    MaterialBindingFixtureStatus last_status = MaterialBindingFixtureStatus::InvalidArgument;
    RenderFixturePassStatus last_pass_status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus last_rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
};
}
