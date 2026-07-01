// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePassSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/Rhi/RhiCommandType.h"
#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 固定容量 fixture pass 计数器 和 last 观察到的 结果。
 */
struct RenderFixturePassSnapshot final {
    std::size_t pass_record_capacity = 0U;
    std::size_t pass_record_count = 0U;
    std::size_t command_capacity = 0U;
    std::size_t required_command_count = 0U;
    std::size_t required_pass_record_count = 0U;
    std::uint64_t executed_pass_count = 0U;
    std::uint64_t completed_pass_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t rhi_failure_count = 0U;
    std::uint64_t command_capacity_rejected_count = 0U;
    std::uint64_t pass_record_capacity_rejected_count = 0U;
    std::size_t last_recorded_command_count = 0U;
    std::size_t last_failed_command_index = 0U;
    yuengine::rhi::RhiCommandType last_failed_command_type = yuengine::rhi::RhiCommandType::BeginFrame;
    std::size_t last_failed_pass_record_index = 0U;
    std::uint32_t last_failed_pass_id = 0U;
    std::size_t last_capacity_entry_command_capacity = 0U;
    std::size_t last_capacity_entry_current_command_count = 0U;
    std::size_t last_capacity_entry_required_command_count = 0U;
    std::size_t last_capacity_entry_pass_record_capacity = 0U;
    std::size_t last_capacity_entry_current_pass_record_count = 0U;
    std::size_t last_capacity_entry_required_pass_record_count = 0U;
    std::size_t last_capacity_entry_failed_command_index = 0U;
    yuengine::rhi::RhiCommandType last_capacity_entry_failed_command_type =
        yuengine::rhi::RhiCommandType::BeginFrame;
    std::size_t last_capacity_entry_failed_pass_record_index = 0U;
    std::uint32_t last_capacity_entry_pass_id = 0U;
    RenderFixturePassStatus last_capacity_entry_status = RenderFixturePassStatus::InvalidArgument;
    std::size_t last_capture_bytes_written = 0U;
    yuengine::rhi::RhiExtent2D last_capture_extent{};
    std::uint32_t last_pass_id = 0U;
    RenderFixturePassStatus last_status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus last_rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
};
}
