// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSubmissionBatchFixtureSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 固定容量 RenderCore submission batch fixture 计数器 和 last 状态 值。
 */
struct RenderSubmissionBatchFixtureSnapshot final {
    std::size_t submission_record_capacity = 0U;
    std::size_t submission_record_count = 0U;
    std::uint64_t accepted_entry_count = 0U;
    std::uint64_t executed_entry_count = 0U;
    std::uint64_t completed_entry_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t duplicate_pass_id_count = 0U;
    std::uint64_t batch_capacity_rejected_count = 0U;
    std::uint64_t render_pass_failure_count = 0U;
    std::size_t last_entry_index = 0U;
    std::size_t last_recorded_command_count = 0U;
    std::size_t last_capture_bytes_written = 0U;
    std::uint32_t last_pass_id = 0U;
    std::uint32_t last_material_id = 0U;
    RenderSubmissionBatchFixtureStatus last_status = RenderSubmissionBatchFixtureStatus::InvalidArgument;
    RenderFixturePassStatus last_pass_status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus last_rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
};
}
