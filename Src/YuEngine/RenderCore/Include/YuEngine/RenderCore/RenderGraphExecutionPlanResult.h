// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlanResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureStatus.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanOperation.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanStatus.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 结果 的 一个 RenderCore render graph execution-plan 操作。
 */
struct RenderGraphExecutionPlanResult final {
    RenderGraphExecutionPlanStatus status = RenderGraphExecutionPlanStatus::InvalidArgument;
    RenderGraphExecutionPlanOperation operation = RenderGraphExecutionPlanOperation::None;
    RenderGraphSkeletonStatus graph_status = RenderGraphSkeletonStatus::InvalidArgument;
    RenderFramePacketFixtureStatus frame_status = RenderFramePacketFixtureStatus::InvalidArgument;
    RenderSubmissionBatchFixtureStatus batch_status = RenderSubmissionBatchFixtureStatus::InvalidArgument;
    RenderFixturePassStatus pass_status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    std::uint32_t plan_id = 0U;
    std::uint32_t graph_id = 0U;
    std::uint32_t frame_id = 0U;
    std::size_t pass_count = 0U;
    std::size_t record_slot = 0U;
    std::size_t plan_record_capacity = 0U;
    std::size_t frame_packet_record_capacity = 0U;
    std::size_t submission_record_capacity = 0U;
    std::size_t current_plan_record_count = 0U;
    std::size_t current_frame_packet_record_count = 0U;
    std::size_t current_submission_record_count = 0U;
    std::size_t required_plan_record_count = 0U;
    std::size_t required_frame_packet_record_count = 0U;
    std::size_t required_submission_record_count = 0U;
    std::size_t completed_entry_count = 0U;
    std::size_t failed_entry_count = 0U;
    std::size_t failed_entry_index = 0U;
    std::uint32_t pass_id = 0U;
    std::uint32_t material_id = 0U;
};
}
