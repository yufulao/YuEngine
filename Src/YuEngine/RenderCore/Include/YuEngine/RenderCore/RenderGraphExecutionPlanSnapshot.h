// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlanSnapshot.h

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
 * @comment Contains bounded RenderCore render graph execution-plan counters and last status values.
 */
struct RenderGraphExecutionPlanSnapshot final {
    std::size_t plan_record_capacity = 0U;
    std::size_t plan_record_count = 0U;
    std::uint64_t accepted_plan_count = 0U;
    std::uint64_t completed_plan_count = 0U;
    std::uint64_t frame_failed_plan_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t duplicate_plan_id_count = 0U;
    std::uint64_t duplicate_graph_execution_count = 0U;
    std::uint64_t plan_capacity_rejected_count = 0U;
    std::uint64_t frame_capacity_rejected_count = 0U;
    std::uint64_t submission_capacity_rejected_count = 0U;
    std::uint64_t query_count = 0U;
    std::uint64_t released_plan_count = 0U;
    std::uint64_t reset_count = 0U;
    std::uint32_t last_plan_id = 0U;
    std::uint32_t last_graph_id = 0U;
    std::uint32_t last_frame_id = 0U;
    std::size_t last_pass_count = 0U;
    std::size_t last_record_slot = 0U;
    std::size_t last_completed_entry_count = 0U;
    std::size_t last_failed_entry_count = 0U;
    std::size_t last_failed_entry_index = 0U;
    std::uint32_t last_pass_id = 0U;
    std::uint32_t last_material_id = 0U;
    RenderGraphExecutionPlanStatus last_status = RenderGraphExecutionPlanStatus::InvalidArgument;
    RenderGraphExecutionPlanOperation last_operation = RenderGraphExecutionPlanOperation::None;
    RenderGraphSkeletonStatus last_graph_status = RenderGraphSkeletonStatus::InvalidArgument;
    RenderFramePacketFixtureStatus last_frame_status = RenderFramePacketFixtureStatus::InvalidArgument;
    RenderSubmissionBatchFixtureStatus last_batch_status = RenderSubmissionBatchFixtureStatus::InvalidArgument;
    RenderFixturePassStatus last_pass_status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus last_rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
};
}
