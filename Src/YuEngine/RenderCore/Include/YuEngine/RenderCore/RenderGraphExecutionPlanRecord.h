// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlanRecord.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureStatus.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 一个 保留的 RenderCore render graph execution-plan metadata 记录。
 */
struct RenderGraphExecutionPlanRecord final {
    std::uint32_t plan_id = 0U;
    std::uint32_t graph_id = 0U;
    std::uint32_t frame_id = 0U;
    std::size_t pass_count = 0U;
    std::size_t record_slot = 0U;
    std::size_t completed_entry_count = 0U;
    std::size_t failed_entry_count = 0U;
    std::size_t failed_entry_index = 0U;
    std::uint32_t pass_id = 0U;
    std::uint32_t material_id = 0U;
    RenderGraphExecutionPlanStatus status = RenderGraphExecutionPlanStatus::InvalidArgument;
    RenderFramePacketFixtureStatus frame_status = RenderFramePacketFixtureStatus::InvalidArgument;
    RenderSubmissionBatchFixtureStatus batch_status = RenderSubmissionBatchFixtureStatus::InvalidArgument;
    RenderFixturePassStatus pass_status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
};
}
