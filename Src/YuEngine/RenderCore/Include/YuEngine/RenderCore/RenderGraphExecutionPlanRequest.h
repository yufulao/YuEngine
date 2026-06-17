// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlanRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderGraphSkeletonResult.h"

namespace yuengine::rendercore {
class RenderFramePacketFixture;
class RenderSubmissionBatchFixture;

/**
 * @comment Describes one caller-owned RenderCore render graph execution-plan operation.
 */
struct RenderGraphExecutionPlanRequest final {
    std::uint32_t plan_id = 0U;
    std::uint32_t frame_id = 0U;
    const RenderGraphSkeletonResult *prepared_graph_result = nullptr;
    RenderFramePacketFixture *frame_packet = nullptr;
    RenderSubmissionBatchFixture *submission_batch = nullptr;
};
}
